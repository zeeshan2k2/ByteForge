// Implements the chunked ByteForge .bfg compressor format.

#include "BFGChunkedCompressor.hpp"

#include "../Analysis/PatternMapWriter.hpp"
#include "../Analysis/RegexSequenceFinder.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>

namespace ByteForge {
namespace {

struct BFGDictionaryPattern {
    unsigned char id;
    std::vector<unsigned char> bytes;
};

struct BFGCompressedChunk {
    std::size_t originalSize;
    std::vector<BFGDictionaryPattern> dictionary;
    std::vector<unsigned char> compressedStream;
};

void writeUInt16LE(std::ofstream& outputFile, std::uint16_t value) {
    outputFile.put(static_cast<char>(value & 0xff));
    outputFile.put(static_cast<char>((value >> 8) & 0xff));
}

void writeUInt64LE(std::ofstream& outputFile, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        outputFile.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

bool startsWithPattern(const std::vector<unsigned char>& bytes,
                       std::size_t startIndex,
                       const std::vector<unsigned char>& pattern) {
    if (startIndex + pattern.size() > bytes.size()) {
        return false;
    }

    for (std::size_t i = 0; i < pattern.size(); ++i) {
        if (bytes[startIndex + i] != pattern[i]) {
            return false;
        }
    }

    return true;
}

std::vector<BFGDictionaryPattern> buildDictionary(const std::vector<BytePatternSummary>& patternSummaries) {
    std::vector<BytePatternSummary> profitablePatterns;

    for (const BytePatternSummary& pattern : patternSummaries) {
        if (pattern.length > 255 || pattern.length < 3 || pattern.occurrences < 2) {
            continue;
        }

        const std::size_t referenceSize = 2;
        const std::size_t dictionaryEntrySize = 2 + pattern.length;
        const std::size_t savedBytes = (pattern.length - referenceSize) * pattern.occurrences;

        if (savedBytes > dictionaryEntrySize) {
            profitablePatterns.push_back(pattern);
        }
    }

    std::sort(profitablePatterns.begin(), profitablePatterns.end(), [](const BytePatternSummary& lhs, const BytePatternSummary& rhs) {
        if (lhs.length != rhs.length) {
            return lhs.length > rhs.length;
        }

        return lhs.occurrences > rhs.occurrences;
    });

    std::vector<BFGDictionaryPattern> dictionary;
    unsigned int nextId = 1;

    for (const BytePatternSummary& pattern : profitablePatterns) {
        if (nextId > 255) {
            break;
        }

        dictionary.push_back(BFGDictionaryPattern{
            static_cast<unsigned char>(nextId),
            std::vector<unsigned char>(pattern.length, pattern.byte)
        });
        ++nextId;
    }

    return dictionary;
}

std::vector<unsigned char> buildCompressedStream(const std::vector<unsigned char>& bytes,
                                                 const std::vector<BFGDictionaryPattern>& dictionary) {
    const unsigned char markerByte = 0xff;
    std::vector<unsigned char> compressedStream;

    for (std::size_t i = 0; i < bytes.size();) {
        const BFGDictionaryPattern* matchedPattern = nullptr;

        for (const BFGDictionaryPattern& pattern : dictionary) {
            if (startsWithPattern(bytes, i, pattern.bytes)) {
                matchedPattern = &pattern;
                break;
            }
        }

        if (matchedPattern != nullptr) {
            compressedStream.push_back(markerByte);
            compressedStream.push_back(matchedPattern->id);
            i += matchedPattern->bytes.size();
            continue;
        }

        if (bytes[i] == markerByte) {
            compressedStream.push_back(markerByte);
            compressedStream.push_back(0x00);
        } else {
            compressedStream.push_back(bytes[i]);
        }

        ++i;
    }

    return compressedStream;
}

BFGCompressedChunk compressChunk(const std::vector<unsigned char>& chunkBytes) {
    const std::vector<ByteRun> runs = RegexSequenceFinder::findRepeatedByteRuns(chunkBytes);
    const std::vector<BytePatternSummary> patterns = PatternMapWriter::summarizeRuns(runs);
    const std::vector<BFGDictionaryPattern> dictionary = buildDictionary(patterns);

    return BFGCompressedChunk{
        chunkBytes.size(),
        dictionary,
        buildCompressedStream(chunkBytes, dictionary)
    };
}

}

bool BFGChunkedCompressor::compress(const std::vector<unsigned char>& bytes,
                                    std::size_t chunkCount,
                                    const std::string& outputPath,
                                    BFGChunkedCompressionResult& result) {
    if (bytes.empty() || chunkCount == 0 || chunkCount > 65535) {
        return false;
    }

    chunkCount = std::min(chunkCount, bytes.size());
    const std::size_t baseChunkSize = bytes.size() / chunkCount;
    const std::size_t extraBytes = bytes.size() % chunkCount;
    std::vector<BFGCompressedChunk> chunks;
    chunks.reserve(chunkCount);

    std::size_t chunkStart = 0;
    for (std::size_t chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex) {
        const std::size_t currentChunkSize = baseChunkSize + (chunkIndex < extraBytes ? 1 : 0);
        const std::size_t chunkEnd = chunkStart + currentChunkSize;
        const std::vector<unsigned char> chunkBytes(bytes.begin() + chunkStart, bytes.begin() + chunkEnd);

        chunks.push_back(compressChunk(chunkBytes));
        chunkStart = chunkEnd;
    }

    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile) {
        return false;
    }

    outputFile.write("BFG2", 4);
    writeUInt64LE(outputFile, static_cast<std::uint64_t>(bytes.size()));
    writeUInt16LE(outputFile, static_cast<std::uint16_t>(chunks.size()));

    std::size_t totalCompressedStreamSize = 0;
    std::size_t totalDictionaryEntries = 0;

    for (const BFGCompressedChunk& chunk : chunks) {
        writeUInt64LE(outputFile, static_cast<std::uint64_t>(chunk.originalSize));
        writeUInt16LE(outputFile, static_cast<std::uint16_t>(chunk.dictionary.size()));

        for (const BFGDictionaryPattern& pattern : chunk.dictionary) {
            outputFile.put(static_cast<char>(pattern.id));
            outputFile.put(static_cast<char>(pattern.bytes.size()));
            outputFile.write(reinterpret_cast<const char*>(pattern.bytes.data()), static_cast<std::streamsize>(pattern.bytes.size()));
        }

        writeUInt64LE(outputFile, static_cast<std::uint64_t>(chunk.compressedStream.size()));
        outputFile.write(reinterpret_cast<const char*>(chunk.compressedStream.data()),
                         static_cast<std::streamsize>(chunk.compressedStream.size()));

        totalCompressedStreamSize += chunk.compressedStream.size();
        totalDictionaryEntries += chunk.dictionary.size();
    }

    outputFile.close();

    result.originalSize = bytes.size();
    result.compressedFileSize = std::filesystem::file_size(outputPath);
    result.chunkCount = chunks.size();
    result.totalCompressedStreamSize = totalCompressedStreamSize;
    result.totalDictionaryEntries = totalDictionaryEntries;

    return true;
}

}
