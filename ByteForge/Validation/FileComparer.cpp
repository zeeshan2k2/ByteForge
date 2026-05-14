// Implements helpers for comparing original and decompressed files byte-by-byte.

#include "FileComparer.hpp"

#include <fstream>
#include <limits>

namespace ByteForge {

FileComparisonResult FileComparer::compare(const std::string& leftPath,
                                           const std::string& rightPath) {
    std::ifstream leftFile(leftPath, std::ios::binary);
    std::ifstream rightFile(rightPath, std::ios::binary);

    if (!leftFile || !rightFile) {
        return FileComparisonResult{false, 0, 0};
    }

    std::size_t offset = 0;
    while (true) {
        const int leftByte = leftFile.get();
        const int rightByte = rightFile.get();

        if (leftByte == EOF && rightByte == EOF) {
            return FileComparisonResult{true, offset, std::numeric_limits<std::size_t>::max()};
        }

        if (leftByte != rightByte) {
            return FileComparisonResult{false, offset, offset};
        }

        ++offset;
    }
}

}
