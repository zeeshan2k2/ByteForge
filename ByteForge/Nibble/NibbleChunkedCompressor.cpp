// Compresses only nibble-friendly byte runs and keeps the rest as raw chunks.

#include "NibbleChunkedCompressor.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace ByteForge {
namespace {

constexpr unsigned char rawChunkType = 0x00;
constexpr unsigned char nibbleChunkType = 0x01;
constexpr std::size_t maxNibbleRunSize = 65535;
constexpr std::size_t minNibbleRunSize = 64;

struct NibbleCandidate {
    std::size_t length;
    std::array<unsigned char, 16> dictionary;
    std::size_t dictionaryCount;
};

void writeUInt16LE(std::ofstream& outputFile, std::uint16_t value) {
    outputFile.put(static_cast<char>(value & 0xff));
    outputFile.put(static_cast<char>((value >> 8) & 0xff));
}

void writeUInt32LE(std::ofstream& outputFile, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        outputFile.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

void writeUInt64LE(std::ofstream& outputFile, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        outputFile.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

std::vector<unsigned char> packNibbles(const std::vector<unsigned char>& nibbles) {
    std::vector<unsigned char> packed;
    packed.reserve((nibbles.size() + 1) / 2);

    for (std::size_t i = 0; i < nibbles.size(); i += 2) {
        const unsigned char highNibble = static_cast<unsigned char>((nibbles[i] & 0x0f) << 4);
        const unsigned char lowNibble = (i + 1 < nibbles.size()) ? static_cast<unsigned char>(nibbles[i + 1] & 0x0f) : 0;
        packed.push_back(static_cast<unsigned char>(highNibble | lowNibble));
    }

    return packed;
}

NibbleCandidate findNibbleCandidate(const std::vector<unsigned char>& bytes, std::size_t startIndex) {
    NibbleCandidate candidate{};
    std::array<int, 256> byteToCode;
    byteToCode.fill(-1);

    const std::size_t endIndex = std::min(bytes.size(), startIndex + maxNibbleRunSize);

    for (std::size_t index = startIndex; index < endIndex; ++index) {
        const unsigned char byte = bytes[index];

        if (byteToCode[byte] < 0) {
            if (candidate.dictionaryCount == candidate.dictionary.size()) {
                break;
            }

            byteToCode[byte] = static_cast<int>(candidate.dictionaryCount);
            candidate.dictionary[candidate.dictionaryCount] = byte;
            ++candidate.dictionaryCount;
        }

        ++candidate.length;
    }

    return candidate;
}

bool isNibbleCandidateWorthIt(const NibbleCandidate& candidate) {
    if (candidate.length < minNibbleRunSize) {
        return false;
    }

    const std::size_t rawChunkOverhead = 1 + 4;
    const std::size_t nibbleChunkOverhead = 1 + 4 + 1 + candidate.dictionaryCount;
    const std::size_t packedNibbleSize = (candidate.length + 1) / 2;

    return nibbleChunkOverhead + packedNibbleSize < rawChunkOverhead + candidate.length;
}

void writeRawChunk(std::ofstream& outputFile,
                   const std::vector<unsigned char>& bytes,
                   std::size_t startIndex,
                   std::size_t length) {
    outputFile.put(static_cast<char>(rawChunkType));
    writeUInt32LE(outputFile, static_cast<std::uint32_t>(length));
    outputFile.write(reinterpret_cast<const char*>(bytes.data() + startIndex),
                     static_cast<std::streamsize>(length));
}

void writeNibbleChunk(std::ofstream& outputFile,
                      const std::vector<unsigned char>& bytes,
                      std::size_t startIndex,
                      const NibbleCandidate& candidate) {
    std::array<int, 256> byteToCode;
    byteToCode.fill(-1);

    for (std::size_t i = 0; i < candidate.dictionaryCount; ++i) {
        byteToCode[candidate.dictionary[i]] = static_cast<int>(i);
    }

    std::vector<unsigned char> nibbles;
    nibbles.reserve(candidate.length);

    for (std::size_t i = 0; i < candidate.length; ++i) {
        nibbles.push_back(static_cast<unsigned char>(byteToCode[bytes[startIndex + i]]));
    }

    const std::vector<unsigned char> packed = packNibbles(nibbles);

    outputFile.put(static_cast<char>(nibbleChunkType));
    writeUInt32LE(outputFile, static_cast<std::uint32_t>(candidate.length));
    outputFile.put(static_cast<char>(candidate.dictionaryCount));

    for (std::size_t i = 0; i < candidate.dictionaryCount; ++i) {
        outputFile.put(static_cast<char>(candidate.dictionary[i]));
    }

    writeUInt16LE(outputFile, static_cast<std::uint16_t>(packed.size()));
    outputFile.write(reinterpret_cast<const char*>(packed.data()),
                     static_cast<std::streamsize>(packed.size()));
}

}

bool NibbleChunkedCompressor::compress(const std::vector<unsigned char>& bytes,
                                       const std::string& outputPath,
                                       NibbleChunkedCompressionResult& result) {
    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile) {
        return false;
    }

    outputFile.write("BFGN2", 5);
    writeUInt64LE(outputFile, static_cast<std::uint64_t>(bytes.size()));

    const std::streampos chunkCountPosition = outputFile.tellp();
    writeUInt32LE(outputFile, 0);

    std::size_t chunkCount = 0;
    std::size_t nibbleChunkCount = 0;
    std::size_t rawChunkCount = 0;
    std::size_t nibbleEncodedBytes = 0;
    std::size_t rawBytes = 0;

    std::size_t pendingRawStart = 0;
    std::size_t pendingRawLength = 0;

    const auto flushRaw = [&]() {
        if (pendingRawLength == 0) {
            return;
        }

        writeRawChunk(outputFile, bytes, pendingRawStart, pendingRawLength);
        ++chunkCount;
        ++rawChunkCount;
        rawBytes += pendingRawLength;
        pendingRawLength = 0;
    };

    for (std::size_t index = 0; index < bytes.size();) {
        const NibbleCandidate candidate = findNibbleCandidate(bytes, index);

        if (isNibbleCandidateWorthIt(candidate)) {
            flushRaw();
            writeNibbleChunk(outputFile, bytes, index, candidate);
            ++chunkCount;
            ++nibbleChunkCount;
            nibbleEncodedBytes += candidate.length;
            index += candidate.length;
            continue;
        }

        if (pendingRawLength == 0) {
            pendingRawStart = index;
        }

        ++pendingRawLength;
        ++index;

        if (pendingRawLength == UINT32_MAX) {
            flushRaw();
        }
    }

    flushRaw();

    outputFile.seekp(chunkCountPosition);
    writeUInt32LE(outputFile, static_cast<std::uint32_t>(chunkCount));
    outputFile.close();

    result.originalSize = bytes.size();
    result.chunkCount = chunkCount;
    result.nibbleChunkCount = nibbleChunkCount;
    result.rawChunkCount = rawChunkCount;
    result.nibbleEncodedBytes = nibbleEncodedBytes;
    result.rawBytes = rawBytes;
    result.compressedFileSize = std::filesystem::file_size(outputPath);

    return true;
}

}
