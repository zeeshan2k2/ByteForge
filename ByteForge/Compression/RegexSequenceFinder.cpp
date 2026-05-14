// Implements helpers for finding repeated byte patterns inside raw byte chunks.

#include "RegexSequenceFinder.hpp"

namespace ByteForge {

std::vector<ByteRun> RegexSequenceFinder::findRepeatedByteRuns(const std::vector<unsigned char>& bytes) {
    std::vector<ByteRun> runs;

    if (bytes.empty()) {
        return runs;
    }

    std::size_t runStart = 0;
    for (std::size_t i = 1; i <= bytes.size(); ++i) {
        if (i < bytes.size() && bytes[i] == bytes[runStart]) {
            continue;
        }

        const std::size_t runLength = i - runStart;
        if (runLength > 1) {
            runs.push_back(ByteRun{bytes[runStart], runStart, runLength});
        }

        runStart = i;
    }

    return runs;
}

ByteRun RegexSequenceFinder::findLargestRun(const std::vector<unsigned char>& bytes) {
    ByteRun largestRun{0, 0, 0};
    const std::vector<ByteRun> runs = findRepeatedByteRuns(bytes);

    for (const ByteRun& run : runs) {
        if (run.length > largestRun.length) {
            largestRun = run;
        }
    }

    return largestRun;
}

}
