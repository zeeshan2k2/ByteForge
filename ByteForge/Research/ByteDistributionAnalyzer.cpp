// Computes byte distribution metrics for analysis and future compression experiments.

#include "ByteDistributionAnalyzer.hpp"

#include <algorithm>
#include <array>

namespace ByteForge {
namespace {

double coveragePercent(std::size_t covered, std::size_t total) {
    if (total == 0) {
        return 0.0;
    }

    return (static_cast<double>(covered) / static_cast<double>(total)) * 100.0;
}

}

ByteDistributionMetrics ByteDistributionAnalyzer::analyze(const std::vector<unsigned char>& bytes) {
    std::array<std::size_t, 256> frequencies{};

    for (const unsigned char byte : bytes) {
        ++frequencies[byte];
    }

    std::size_t distinctByteCount = 0;
    std::size_t zeroTo3Count = 0;
    std::size_t fourTo15Count = 0;
    std::size_t sixteenTo63Count = 0;
    std::size_t sixtyFourTo127Count = 0;
    std::size_t oneTwentyEightTo255Count = 0;

    std::vector<std::size_t> nonZeroFrequencies;
    nonZeroFrequencies.reserve(256);

    for (std::size_t value = 0; value < frequencies.size(); ++value) {
        const std::size_t frequency = frequencies[value];
        if (frequency == 0) {
            continue;
        }

        ++distinctByteCount;
        nonZeroFrequencies.push_back(frequency);

        if (value < 4) {
            zeroTo3Count += frequency;
        } else if (value < 16) {
            fourTo15Count += frequency;
        } else if (value < 64) {
            sixteenTo63Count += frequency;
        } else if (value < 128) {
            sixtyFourTo127Count += frequency;
        } else {
            oneTwentyEightTo255Count += frequency;
        }
    }

    std::sort(nonZeroFrequencies.begin(), nonZeroFrequencies.end(), std::greater<>());

    auto topCoverage = [&](std::size_t count) {
        const std::size_t cappedCount = std::min(count, nonZeroFrequencies.size());
        std::size_t covered = 0;

        for (std::size_t i = 0; i < cappedCount; ++i) {
            covered += nonZeroFrequencies[i];
        }

        return coveragePercent(covered, bytes.size());
    };

    return ByteDistributionMetrics{
        bytes.size(),
        distinctByteCount,
        zeroTo3Count,
        fourTo15Count,
        sixteenTo63Count,
        sixtyFourTo127Count,
        oneTwentyEightTo255Count,
        topCoverage(4),
        topCoverage(16),
        topCoverage(64)
    };
}

}
