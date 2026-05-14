// Implements the chunked ByteForge .bfg decompressor.

#include "BFGChunkedDecompressor.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

namespace ByteForge {
namespace {

std::uint16_t readUInt16LE(std::ifstream& inputFile) {
    const unsigned char low = static_cast<unsigned char>(inputFile.get());
    const unsigned char high = static_cast<unsigned char>(inputFile.get());

    return static_cast<std::uint16_t>(low | (high << 8));
}

std::uint64_t readUInt64LE(std::ifstream& inputFile) {
    std::uint64_t value = 0;

    for (int i = 0; i < 8; ++i) {
        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(inputFile.get())) << (i * 8);
    }

    return value;
}

bool readExact(std::ifstream& inputFile, char* destination, std::streamsize size) {
    inputFile.read(destination, size);
    return inputFile.gcount() == size;
}

bool decompressStream(const std::vector<unsigned char>& compressedStream,
                      const std::vector<std::vector<unsigned char>>& dictionary,
                      std::vector<unsigned char>& outputBytes) {
    const unsigned char markerByte = 0xff;

    for (std::size_t i = 0; i < compressedStream.size(); ++i) {
        if (compressedStream[i] != markerByte) {
            outputBytes.push_back(compressedStream[i]);
            continue;
        }

        if (i + 1 >= compressedStream.size()) {
            return false;
        }

        const unsigned char key = compressedStream[++i];
        if (key == 0x00) {
            outputBytes.push_back(markerByte);
            continue;
        }

        const std::vector<unsigned char>& pattern = dictionary[key];
        if (pattern.empty()) {
            return false;
        }

        outputBytes.insert(outputBytes.end(), pattern.begin(), pattern.end());
    }

    return true;
}

}

bool BFGChunkedDecompressor::decompress(const std::string& inputPath,
                                        const std::string& outputPath,
                                        BFGChunkedDecompressionResult& result) {
    std::ifstream inputFile(inputPath, std::ios::binary);
    if (!inputFile) {
        return false;
    }

    std::array<char, 4> magic{};
    if (!readExact(inputFile, magic.data(), static_cast<std::streamsize>(magic.size())) ||
        magic[0] != 'B' || magic[1] != 'F' || magic[2] != 'G' || magic[3] != '2') {
        return false;
    }

    const std::uint64_t originalSize = readUInt64LE(inputFile);
    const std::uint16_t chunkCount = readUInt16LE(inputFile);

    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile) {
        return false;
    }

    std::size_t decompressedSize = 0;
    std::size_t totalCompressedStreamSize = 0;
    std::size_t totalDictionaryEntries = 0;

    for (std::uint16_t chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex) {
        const std::uint64_t originalChunkSize = readUInt64LE(inputFile);
        const std::uint16_t dictionaryCount = readUInt16LE(inputFile);
        std::vector<std::vector<unsigned char>> dictionary(256);

        for (std::uint16_t i = 0; i < dictionaryCount; ++i) {
            const unsigned char patternId = static_cast<unsigned char>(inputFile.get());
            const unsigned char patternLength = static_cast<unsigned char>(inputFile.get());

            if (!inputFile || patternId == 0 || patternLength == 0) {
                return false;
            }

            std::vector<unsigned char> pattern(patternLength);
            if (!readExact(inputFile, reinterpret_cast<char*>(pattern.data()), patternLength)) {
                return false;
            }

            dictionary[patternId] = pattern;
        }

        const std::uint64_t compressedStreamSize = readUInt64LE(inputFile);
        std::vector<unsigned char> compressedStream(compressedStreamSize);
        if (!readExact(inputFile,
                       reinterpret_cast<char*>(compressedStream.data()),
                       static_cast<std::streamsize>(compressedStream.size()))) {
            return false;
        }

        std::vector<unsigned char> chunkBytes;
        chunkBytes.reserve(static_cast<std::size_t>(originalChunkSize));
        if (!decompressStream(compressedStream, dictionary, chunkBytes) ||
            chunkBytes.size() != originalChunkSize) {
            return false;
        }

        outputFile.write(reinterpret_cast<const char*>(chunkBytes.data()),
                         static_cast<std::streamsize>(chunkBytes.size()));

        decompressedSize += chunkBytes.size();
        totalCompressedStreamSize += compressedStream.size();
        totalDictionaryEntries += dictionaryCount;
    }

    if (decompressedSize != originalSize) {
        return false;
    }

    result.originalSize = static_cast<std::size_t>(originalSize);
    result.decompressedSize = decompressedSize;
    result.chunkCount = chunkCount;
    result.totalCompressedStreamSize = totalCompressedStreamSize;
    result.totalDictionaryEntries = totalDictionaryEntries;

    return true;
}

}
