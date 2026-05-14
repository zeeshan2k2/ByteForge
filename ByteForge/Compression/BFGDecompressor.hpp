// Declares the ByteForge .bfg decompressor for rebuilding original byte files.

#ifndef BFGDecompressor_hpp
#define BFGDecompressor_hpp

#include <cstddef>
#include <string>

namespace ByteForge {

struct BFGDecompressionResult {
    std::size_t originalSize;
    std::size_t compressedStreamSize;
    std::size_t decompressedSize;
    std::size_t dictionaryCount;
};

class BFGDecompressor {
public:
    static bool decompress(const std::string& inputPath,
                           const std::string& outputPath,
                           BFGDecompressionResult& result);
};

}

#endif
