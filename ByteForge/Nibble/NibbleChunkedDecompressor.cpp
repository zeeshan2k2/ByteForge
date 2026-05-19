// Rebuilds bytes from the BFGN2 chunked nibble/raw fallback format.

#include "NibbleChunkedDecompressor.hpp"

#include <array>
#include <fstream>
#include <vector>

#include "../Core/ByteWriter.hpp"

namespace ByteForge {
namespace {

constexpr unsigned char rawChunkType = 0x00;
constexpr unsigned char nibbleChunkType = 0x01;

bool readUInt16LE(std::ifstream& inputFile, std::uint16_t& value) {
    value = 0;

    for (int i = 0; i < 2; ++i) {
        const int byte = inputFile.get();
        if (byte == EOF) {
            return false;
        }

        value |= static_cast<std::uint16_t>(static_cast<unsigned char>(byte)) << (i * 8);
    }

    return true;
}

bool readUInt32LE(std::ifstream& inputFile, std::uint32_t& value) {
    value = 0;

    for (int i = 0; i < 4; ++i) {
        const int byte = inputFile.get();
        if (byte == EOF) {
            return false;
        }

        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(byte)) << (i * 8);
    }

    return true;
}

bool readUInt64LE(std::ifstream& inputFile, std::uint64_t& value) {
    value = 0;

    for (int i = 0; i < 8; ++i) {
        const int byte = inputFile.get();
        if (byte == EOF) {
            return false;
        }

        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(byte)) << (i * 8);
    }

    return true;
}

std::vector<unsigned char> unpackNibbles(const std::vector<unsigned char>& packed, std::size_t nibbleCount) {
    std::vector<unsigned char> nibbles;
    nibbles.reserve(nibbleCount);

    for (const unsigned char byte : packed) {
        if (nibbles.size() < nibbleCount) {
            nibbles.push_back(static_cast<unsigned char>((byte >> 4) & 0x0f));
        }

        if (nibbles.size() < nibbleCount) {
            nibbles.push_back(static_cast<unsigned char>(byte & 0x0f));
        }
    }

    return nibbles;
}

}

bool NibbleChunkedDecompressor::decompress(const std::string& inputPath,
                                           const std::string& outputPath,
                                           NibbleChunkedDecompressionResult& result) {
    std::ifstream inputFile(inputPath, std::ios::binary);
    if (!inputFile) {
        return false;
    }

    std::array<char, 5> magic{};
    inputFile.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (inputFile.gcount() != static_cast<std::streamsize>(magic.size()) ||
        std::string(magic.data(), magic.size()) != "BFGN2") {
        return false;
    }

    std::uint64_t originalSize = 0;
    std::uint32_t chunkCount = 0;

    if (!readUInt64LE(inputFile, originalSize) || !readUInt32LE(inputFile, chunkCount)) {
        return false;
    }

    std::vector<unsigned char> rebuilt;
    rebuilt.reserve(static_cast<std::size_t>(originalSize));

    std::size_t nibbleChunkCount = 0;
    std::size_t rawChunkCount = 0;

    for (std::uint32_t chunkIndex = 0; chunkIndex < chunkCount; ++chunkIndex) {
        const int chunkTypeValue = inputFile.get();
        if (chunkTypeValue == EOF) {
            return false;
        }

        std::uint32_t chunkSize = 0;
        if (!readUInt32LE(inputFile, chunkSize)) {
            return false;
        }

        const unsigned char chunkType = static_cast<unsigned char>(chunkTypeValue);

        if (chunkType == rawChunkType) {
            std::vector<unsigned char> rawBytes(chunkSize);
            if (!rawBytes.empty()) {
                inputFile.read(reinterpret_cast<char*>(rawBytes.data()),
                               static_cast<std::streamsize>(rawBytes.size()));
                if (inputFile.gcount() != static_cast<std::streamsize>(rawBytes.size())) {
                    return false;
                }
            }

            rebuilt.insert(rebuilt.end(), rawBytes.begin(), rawBytes.end());
            ++rawChunkCount;
            continue;
        }

        if (chunkType != nibbleChunkType) {
            return false;
        }

        const int dictionaryCountValue = inputFile.get();
        if (dictionaryCountValue == EOF || dictionaryCountValue < 1 || dictionaryCountValue > 16) {
            return false;
        }

        const std::size_t dictionaryCount = static_cast<std::size_t>(dictionaryCountValue);
        std::array<unsigned char, 16> dictionary{};

        for (std::size_t i = 0; i < dictionaryCount; ++i) {
            const int byte = inputFile.get();
            if (byte == EOF) {
                return false;
            }

            dictionary[i] = static_cast<unsigned char>(byte);
        }

        std::uint16_t packedSize = 0;
        if (!readUInt16LE(inputFile, packedSize)) {
            return false;
        }

        std::vector<unsigned char> packed(packedSize);
        if (!packed.empty()) {
            inputFile.read(reinterpret_cast<char*>(packed.data()),
                           static_cast<std::streamsize>(packed.size()));
            if (inputFile.gcount() != static_cast<std::streamsize>(packed.size())) {
                return false;
            }
        }

        const std::vector<unsigned char> nibbles = unpackNibbles(packed, chunkSize);

        for (const unsigned char nibble : nibbles) {
            if (nibble >= dictionaryCount) {
                return false;
            }

            rebuilt.push_back(dictionary[nibble]);
        }

        ++nibbleChunkCount;
    }

    if (rebuilt.size() != static_cast<std::size_t>(originalSize)) {
        return false;
    }

    if (!ByteWriter::writeBytes(outputPath, rebuilt)) {
        return false;
    }

    result.decompressedSize = rebuilt.size();
    result.chunkCount = chunkCount;
    result.nibbleChunkCount = nibbleChunkCount;
    result.rawChunkCount = rawChunkCount;

    return true;
}

}
