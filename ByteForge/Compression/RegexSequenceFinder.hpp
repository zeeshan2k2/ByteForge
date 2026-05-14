// Declares helpers for finding repeated byte patterns inside raw byte chunks.

#ifndef RegexSequenceFinder_hpp
#define RegexSequenceFinder_hpp

#include <cstddef>
#include <vector>

namespace ByteForge {

struct ByteRun {
    unsigned char byte;
    std::size_t startIndex;
    std::size_t length;
};

class RegexSequenceFinder {
public:
    static std::vector<ByteRun> findRepeatedByteRuns(const std::vector<unsigned char>& bytes);
    static ByteRun findLargestRun(const std::vector<unsigned char>& bytes);
};

}

#endif
