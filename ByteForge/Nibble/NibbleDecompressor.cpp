// Rebuilds original bytes from the single-dictionary BFGN nibble format.

#include "NibbleDecompressor.hpp"

#include <array>
#include <fstream>
#include <vector>

#include "../Core/ByteWriter.hpp"

namespace ByteForge {
namespace {

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

std::vector<unsigned char> unpackNibbles(const std::vector<unsigned char>& packedStream,
                                         std::size_t nibbleCount) {
    std::vector<unsigned char> nibbles;
    nibbles.reserve(nibbleCount);

    for (const unsigned char byte : packedStream) {
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

bool NibbleDecompressor::decompress(const std::string& inputPath,
                                    const std::string& outputPath,
                                    NibbleDecompressionResult& result) {
    std::ifstream inputFile(inputPath, std::ios::binary);
    if (!inputFile) {
        return false;
    }

    std::array<char, 5> magic{};
    inputFile.read(magic.data(), static_cast<std::streamsize>(magic.size()));
    if (inputFile.gcount() != static_cast<std::streamsize>(magic.size()) ||
        std::string(magic.data(), magic.size()) != "BFGN1") {
        return false;
    }

    std::uint64_t originalSize = 0;
    if (!readUInt64LE(inputFile, originalSize)) {
        return false;
    }

    const int dictionaryCountValue = inputFile.get();
    if (dictionaryCountValue == EOF ||
        dictionaryCountValue < 0 ||
        dictionaryCountValue > static_cast<int>(BFGN_MAX_DICTIONARY_ENTRIES)) {
        return false;
    }

    const std::size_t dictionaryCount = static_cast<std::size_t>(dictionaryCountValue);
    std::array<unsigned char, BFGN_MAX_DICTIONARY_ENTRIES> dictionary{};

    for (std::size_t i = 0; i < dictionaryCount; ++i) {
        const int byte = inputFile.get();
        if (byte == EOF) {
            return false;
        }

        dictionary[i] = static_cast<unsigned char>(byte);
    }

    std::uint64_t compressedNibbleCount = 0;
    if (!readUInt64LE(inputFile, compressedNibbleCount)) {
        return false;
    }

    const std::size_t packedStreamSize = static_cast<std::size_t>((compressedNibbleCount + 1) / 2);
    std::vector<unsigned char> packedStream(packedStreamSize);

    if (!packedStream.empty()) {
        inputFile.read(reinterpret_cast<char*>(packedStream.data()),
                       static_cast<std::streamsize>(packedStream.size()));
        if (inputFile.gcount() != static_cast<std::streamsize>(packedStream.size())) {
            return false;
        }
    }

    const std::vector<unsigned char> nibbles = unpackNibbles(packedStream, static_cast<std::size_t>(compressedNibbleCount));
    std::vector<unsigned char> rebuilt;
    rebuilt.reserve(static_cast<std::size_t>(originalSize));

    for (std::size_t i = 0; i < nibbles.size();) {
        const unsigned char code = nibbles[i++];

        if (code == BFGN_ESCAPE_NIBBLE) {
            if (i + 1 >= nibbles.size()) {
                return false;
            }

            const unsigned char highNibble = nibbles[i++];
            const unsigned char lowNibble = nibbles[i++];
            rebuilt.push_back(static_cast<unsigned char>((highNibble << 4) | lowNibble));
            continue;
        }

        if (code >= dictionaryCount) {
            return false;
        }

        rebuilt.push_back(dictionary[code]);
    }

    if (rebuilt.size() != static_cast<std::size_t>(originalSize)) {
        return false;
    }

    if (!ByteWriter::writeBytes(outputPath, rebuilt)) {
        return false;
    }

    result.decompressedSize = rebuilt.size();
    result.dictionaryCount = dictionaryCount;
    result.compressedNibbleCount = static_cast<std::size_t>(compressedNibbleCount);

    return true;
}

}
