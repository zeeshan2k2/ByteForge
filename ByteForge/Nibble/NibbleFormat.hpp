// Defines the shared structs and constants for the BFGN nibble dictionary format.

#ifndef NibbleFormat_hpp
#define NibbleFormat_hpp

#include <cstddef>
#include <cstdint>

namespace ByteForge {

constexpr unsigned char BFGN_ESCAPE_NIBBLE = 0x0f;
constexpr std::size_t BFGN_MAX_DICTIONARY_ENTRIES = 15;

struct NibbleDictionaryEntry {
    unsigned char code;
    unsigned char byte;
    std::size_t frequency;
};

struct NibbleCompressionResult {
    std::size_t originalSize;
    std::size_t dictionaryCount;
    std::size_t mappedBytes;
    std::size_t escapedBytes;
    std::size_t compressedNibbleCount;
    std::size_t compressedStreamSize;
    std::size_t compressedFileSize;
};

struct NibbleDecompressionResult {
    std::size_t decompressedSize;
    std::size_t dictionaryCount;
    std::size_t compressedNibbleCount;
};

}

#endif
