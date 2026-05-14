// Implements helpers for reading raw bytes from source files.

#include "ByteReader.hpp"

#include <fstream>

namespace ByteForge {

bool ByteReader::readBytes(const std::string& filePath,
                           std::size_t bytesToRead,
                           std::vector<unsigned char>& bytes) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) {
        return false;
    }

    bytes.resize(bytesToRead);
    inputFile.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    bytes.resize(static_cast<std::size_t>(inputFile.gcount()));

    return true;
}

bool ByteReader::readAllBytes(const std::string& filePath,
                              std::vector<unsigned char>& bytes) {
    std::ifstream inputFile(filePath, std::ios::binary | std::ios::ate);
    if (!inputFile) {
        return false;
    }

    const std::streamsize fileSize = inputFile.tellg();
    if (fileSize < 0) {
        return false;
    }

    inputFile.seekg(0, std::ios::beg);
    bytes.resize(static_cast<std::size_t>(fileSize));

    if (bytes.empty()) {
        return true;
    }

    inputFile.read(reinterpret_cast<char*>(bytes.data()), fileSize);
    return inputFile.gcount() == fileSize;
}

}
