// Declares the chunked ByteForge .bfg compressor format.

#ifndef BFGChunkedCompressor_hpp
#define BFGChunkedCompressor_hpp

#include <cstddef>
#include <string>
#include <vector>

namespace ByteForge {

struct BFGChunkedCompressionResult {
    std::size_t originalSize;
    std::size_t compressedFileSize;
    std::size_t chunkCount;
    std::size_t totalCompressedStreamSize;
    std::size_t totalDictionaryEntries;
};

class BFGChunkedCompressor {
public:
    static bool compress(const std::vector<unsigned char>& bytes,
                         std::size_t chunkCount,
                         const std::string& outputPath,
                         BFGChunkedCompressionResult& result);
};

}

#endif
