// Implements helpers for writing raw bytes to output files.

#include "ByteWriter.hpp"

#include <fstream>

namespace ByteForge {

bool ByteWriter::writeBytes(const std::string& filePath,
                            const std::vector<unsigned char>& bytes) {
    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) {
        return false;
    }

    if (!bytes.empty()) {
        outputFile.write(reinterpret_cast<const char*>(bytes.data()),
                         static_cast<std::streamsize>(bytes.size()));
    }

    return static_cast<bool>(outputFile);
}

}
