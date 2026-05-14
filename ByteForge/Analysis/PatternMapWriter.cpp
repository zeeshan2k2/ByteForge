// Implements helpers for writing repeated byte patterns into a ByteForge map file.

#include "PatternMapWriter.hpp"

#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <utility>

namespace ByteForge {

std::vector<BytePatternSummary> PatternMapWriter::summarizeRuns(const std::vector<ByteRun>& runs) {
    std::map<std::pair<unsigned int, std::size_t>, std::size_t> counts;

    for (const ByteRun& run : runs) {
        counts[{run.byte, run.length}] += 1;
    }

    std::vector<BytePatternSummary> patterns;
    for (const auto& item : counts) {
        patterns.push_back(BytePatternSummary{
            static_cast<unsigned char>(item.first.first),
            item.first.second,
            item.second
        });
    }

    std::sort(patterns.begin(), patterns.end(), [](const BytePatternSummary& lhs, const BytePatternSummary& rhs) {
        const std::size_t lhsTotalBytes = lhs.length * lhs.occurrences;
        const std::size_t rhsTotalBytes = rhs.length * rhs.occurrences;

        if (lhsTotalBytes != rhsTotalBytes) {
            return lhsTotalBytes > rhsTotalBytes;
        }

        if (lhs.length != rhs.length) {
            return lhs.length > rhs.length;
        }

        return lhs.byte < rhs.byte;
    });

    return patterns;
}

bool PatternMapWriter::writeTextMap(const std::string& outputPath, const std::vector<BytePatternSummary>& patterns) {
    std::ofstream outputFile(outputPath);

    if (!outputFile) {
        return false;
    }

    outputFile << "BFGMAP1\n";
    outputFile << "byte,length,occurrences,total_original_bytes\n";

    for (const BytePatternSummary& pattern : patterns) {
        outputFile << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(pattern.byte)
                   << std::dec << ','
                   << pattern.length << ','
                   << pattern.occurrences << ','
                   << pattern.length * pattern.occurrences << '\n';
    }

    return true;
}

}
