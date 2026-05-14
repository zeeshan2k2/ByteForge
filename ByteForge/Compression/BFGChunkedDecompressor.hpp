// Declares the chunked ByteForge .bfg decompressor.

#ifndef BFGChunkedDecompressor_hpp
#define BFGChunkedDecompressor_hpp

#include <cstddef>
#include <string>

namespace ByteForge {

struct BFGChunkedDecompressionResult {
    std::size_t originalSize;
    std::size_t decompressedSize;
    std::size_t chunkCount;
    std::size_t totalCompressedStreamSize;
    std::size_t totalDictionaryEntries;
};

class BFGChunkedDecompressor {
public:
    static bool decompress(const std::string& inputPath,
                           const std::string& outputPath,
                           BFGChunkedDecompressionResult& result);
};

}

#endif
