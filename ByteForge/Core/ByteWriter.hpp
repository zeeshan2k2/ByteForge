// Declares helpers for writing raw bytes to output files.

#ifndef ByteWriter_hpp
#define ByteWriter_hpp

#include <string>
#include <vector>

namespace ByteForge {

class ByteWriter {
public:
    static bool writeBytes(const std::string& filePath,
                           const std::vector<unsigned char>& bytes);
};

}

#endif
