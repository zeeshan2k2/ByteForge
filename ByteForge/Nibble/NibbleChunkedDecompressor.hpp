// Rebuilds bytes from the BFGN2 chunked nibble/raw fallback format.

#ifndef NibbleChunkedDecompressor_hpp
#define NibbleChunkedDecompressor_hpp

#include <cstddef>
#include <string>

namespace ByteForge {

struct NibbleChunkedDecompressionResult {
    std::size_t decompressedSize;
    std::size_t chunkCount;
    std::size_t nibbleChunkCount;
    std::size_t rawChunkCount;
};

class NibbleChunkedDecompressor {
public:
    static bool decompress(const std::string& inputPath,
                           const std::string& outputPath,
                           NibbleChunkedDecompressionResult& result);
};

}

#endif
