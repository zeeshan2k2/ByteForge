// Compresses bytes with a single 4-bit dictionary and writes the BFGN format.

#ifndef NibbleCompressor_hpp
#define NibbleCompressor_hpp

#include <string>
#include <vector>

#include "NibbleFormat.hpp"

namespace ByteForge {

class NibbleCompressor {
public:
    static bool compress(const std::vector<unsigned char>& bytes,
                         const std::string& outputPath,
                         NibbleCompressionResult& result);
};

}

#endif
