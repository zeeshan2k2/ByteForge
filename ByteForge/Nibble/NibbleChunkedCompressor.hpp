// Compresses only nibble-friendly byte runs and keeps the rest as raw chunks.

#ifndef NibbleChunkedCompressor_hpp
#define NibbleChunkedCompressor_hpp

#include <cstddef>
#include <string>
#include <vector>

namespace ByteForge {

struct NibbleChunkedCompressionResult {
    std::size_t originalSize;
    std::size_t chunkCount;
    std::size_t nibbleChunkCount;
    std::size_t rawChunkCount;
    std::size_t nibbleEncodedBytes;
    std::size_t rawBytes;
    std::size_t compressedFileSize;
};

class NibbleChunkedCompressor {
public:
    static bool compress(const std::vector<unsigned char>& bytes,
                         const std::string& outputPath,
                         NibbleChunkedCompressionResult& result);
};

}

#endif
