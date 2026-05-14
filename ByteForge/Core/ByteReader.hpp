// Declares helpers for reading raw bytes from source files.

#ifndef ByteReader_hpp
#define ByteReader_hpp

#include <cstddef>
#include <string>
#include <vector>

namespace ByteForge {

class ByteReader {
public:
    static bool readBytes(const std::string& filePath,
                          std::size_t bytesToRead,
                          std::vector<unsigned char>& bytes);
    static bool readAllBytes(const std::string& filePath,
                             std::vector<unsigned char>& bytes);
};

}

#endif
