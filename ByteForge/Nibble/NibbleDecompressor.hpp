// Rebuilds original bytes from the single-dictionary BFGN nibble format.

#ifndef NibbleDecompressor_hpp
#define NibbleDecompressor_hpp

#include <string>

#include "NibbleFormat.hpp"

namespace ByteForge {

class NibbleDecompressor {
public:
    static bool decompress(const std::string& inputPath,
                           const std::string& outputPath,
                           NibbleDecompressionResult& result);
};

}

#endif
