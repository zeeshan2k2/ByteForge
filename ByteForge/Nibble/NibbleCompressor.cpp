// Compresses bytes with a single 4-bit dictionary and writes the BFGN format.

#include "NibbleCompressor.hpp"

#include <array>
#include <filesystem>
#include <fstream>

#include "NibbleDictionaryBuilder.hpp"

namespace ByteForge {
namespace {

void writeUInt64LE(std::ofstream& outputFile, std::uint64_t value) {
    for (int i = 0; i < 8; ++i) {
        outputFile.put(static_cast<char>((value >> (i * 8)) & 0xff));
    }
}

void pushByteAsNibbles(std::vector<unsigned char>& nibbles, unsigned char byte) {
    nibbles.push_back(static_cast<unsigned char>((byte >> 4) & 0x0f));
    nibbles.push_back(static_cast<unsigned char>(byte & 0x0f));
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

}

bool NibbleCompressor::compress(const std::vector<unsigned char>& bytes,
                                const std::string& outputPath,
                                NibbleCompressionResult& result) {
    const std::vector<NibbleDictionaryEntry> dictionary = NibbleDictionaryBuilder::build(bytes);
    std::array<int, 256> byteToCode;
    byteToCode.fill(-1);

    for (const NibbleDictionaryEntry& entry : dictionary) {
        byteToCode[entry.byte] = entry.code;
    }

    std::vector<unsigned char> nibbles;
    nibbles.reserve(bytes.size());

    std::size_t mappedBytes = 0;
    std::size_t escapedBytes = 0;

    for (const unsigned char byte : bytes) {
        const int code = byteToCode[byte];
        if (code >= 0) {
            nibbles.push_back(static_cast<unsigned char>(code));
            ++mappedBytes;
            continue;
        }

        nibbles.push_back(BFGN_ESCAPE_NIBBLE);
        pushByteAsNibbles(nibbles, byte);
        ++escapedBytes;
    }

    const std::vector<unsigned char> packedStream = packNibbles(nibbles);

    std::ofstream outputFile(outputPath, std::ios::binary);
    if (!outputFile) {
        return false;
    }

    outputFile.write("BFGN1", 5);
    writeUInt64LE(outputFile, static_cast<std::uint64_t>(bytes.size()));
    outputFile.put(static_cast<char>(dictionary.size()));

    for (const NibbleDictionaryEntry& entry : dictionary) {
        outputFile.put(static_cast<char>(entry.byte));
    }

    writeUInt64LE(outputFile, static_cast<std::uint64_t>(nibbles.size()));

    if (!packedStream.empty()) {
        outputFile.write(reinterpret_cast<const char*>(packedStream.data()),
                         static_cast<std::streamsize>(packedStream.size()));
    }

    outputFile.close();

    result.originalSize = bytes.size();
    result.dictionaryCount = dictionary.size();
    result.mappedBytes = mappedBytes;
    result.escapedBytes = escapedBytes;
    result.compressedNibbleCount = nibbles.size();
    result.compressedStreamSize = packedStream.size();
    result.compressedFileSize = std::filesystem::file_size(outputPath);

    return true;
}

}
