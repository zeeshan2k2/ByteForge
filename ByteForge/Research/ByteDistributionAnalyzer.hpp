// Computes byte distribution metrics for analysis and future compression experiments.

#ifndef ByteDistributionAnalyzer_hpp
#define ByteDistributionAnalyzer_hpp

#include <cstddef>
#include <vector>

namespace ByteForge {

struct ByteDistributionMetrics {
    std::size_t totalBytes;
    std::size_t distinctByteCount;
    std::size_t zeroTo3Count;
    std::size_t fourTo15Count;
    std::size_t sixteenTo63Count;
    std::size_t sixtyFourTo127Count;
    std::size_t oneTwentyEightTo255Count;
    double top4Coverage;
    double top16Coverage;
    double top64Coverage;
};

class ByteDistributionAnalyzer {
public:
    static ByteDistributionMetrics analyze(const std::vector<unsigned char>& bytes);
};

}

#endif
