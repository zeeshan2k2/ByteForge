// Declares helpers for writing repeated byte patterns into a ByteForge map file.

#ifndef PatternMapWriter_hpp
#define PatternMapWriter_hpp

#include <cstddef>
#include <string>
#include <vector>

#include "RegexSequenceFinder.hpp"

namespace ByteForge {

struct BytePatternSummary {
    unsigned char byte;
    std::size_t length;
    std::size_t occurrences;
};

class PatternMapWriter {
public:
    static std::vector<BytePatternSummary> summarizeRuns(const std::vector<ByteRun>& runs);
    static bool writeTextMap(const std::string& outputPath, const std::vector<BytePatternSummary>& patterns);
};

}

#endif
