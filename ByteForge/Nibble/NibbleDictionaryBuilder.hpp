// Builds the single nibble dictionary from the most frequent byte values.

#ifndef NibbleDictionaryBuilder_hpp
#define NibbleDictionaryBuilder_hpp

#include <vector>

#include "NibbleFormat.hpp"

namespace ByteForge {

class NibbleDictionaryBuilder {
public:
    static std::vector<NibbleDictionaryEntry> build(const std::vector<unsigned char>& bytes);
};

}

#endif
