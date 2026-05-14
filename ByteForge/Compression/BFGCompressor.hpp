// Declares the ByteForge .bfg compressor and its binary output format.

#ifndef BFGCompressor_hpp
#define BFGCompressor_hpp

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../Analysis/PatternMapWriter.hpp"

namespace ByteForge {

struct BFGCompressionResult {
    std::size_t originalSize;
    std::size_t compressedStreamSize;
    std::size_t compressedFileSize;
    std::size_t dictionaryCount;
};

class BFGCompressor {
public:
    static bool compress(const std::vector<unsigned char>& bytes,
                         const std::vector<BytePatternSummary>& patternSummaries,
                         const std::string& outputPath,
                         BFGCompressionResult& result);
};

}

#endif
