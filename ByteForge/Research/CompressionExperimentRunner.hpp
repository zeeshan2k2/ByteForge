// Runs ByteForge compression experiments and returns UI-friendly result structs.

#ifndef CompressionExperimentRunner_hpp
#define CompressionExperimentRunner_hpp

#include <cstddef>
#include <string>

#include "ByteDistributionAnalyzer.hpp"

namespace ByteForge {

enum class CompressionMethod {
    RepeatedByte,
    AdaptiveNibble
};

struct CompressionExperimentResult {
    bool success;
    std::string errorMessage;
    std::size_t originalSize;
    std::size_t compressedSize;
    double savedPercent;
    double compressionMilliseconds;
    double decompressionMilliseconds;
    double totalMilliseconds;
    std::size_t chunkCount;
    std::size_t nibbleChunkCount;
    std::size_t rawChunkCount;
    bool rebuildMatches;
    ByteDistributionMetrics distribution;
};

class CompressionExperimentRunner {
public:
    static CompressionExperimentResult run(CompressionMethod method,
                                           const std::string& inputPath,
                                           std::size_t bytesToRead,
                                           const std::string& outputDirectory);
};

}

#endif
