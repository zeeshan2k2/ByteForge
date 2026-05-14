// Declares helpers for comparing original and decompressed files byte-by-byte.

#ifndef FileComparer_hpp
#define FileComparer_hpp

#include <cstddef>
#include <string>

namespace ByteForge {

struct FileComparisonResult {
    bool matches;
    std::size_t bytesCompared;
    std::size_t firstMismatchOffset;
};

class FileComparer {
public:
    static FileComparisonResult compare(const std::string& leftPath,
                                        const std::string& rightPath);
};

}

#endif
