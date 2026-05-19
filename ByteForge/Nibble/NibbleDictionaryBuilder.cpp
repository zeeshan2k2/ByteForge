// Builds the single nibble dictionary from the most frequent byte values.

#include "NibbleDictionaryBuilder.hpp"

#include <algorithm>
#include <array>

namespace ByteForge {

std::vector<NibbleDictionaryEntry> NibbleDictionaryBuilder::build(const std::vector<unsigned char>& bytes) {
    std::array<std::size_t, 256> frequencies{};

    for (const unsigned char byte : bytes) {
        ++frequencies[byte];
    }

    std::vector<NibbleDictionaryEntry> entries;
    entries.reserve(256);

    for (std::size_t byte = 0; byte < frequencies.size(); ++byte) {
        if (frequencies[byte] == 0) {
            continue;
        }

        entries.push_back(NibbleDictionaryEntry{
            0,
            static_cast<unsigned char>(byte),
            frequencies[byte]
        });
    }

    std::sort(entries.begin(), entries.end(), [](const NibbleDictionaryEntry& lhs, const NibbleDictionaryEntry& rhs) {
        if (lhs.frequency != rhs.frequency) {
            return lhs.frequency > rhs.frequency;
        }

        return lhs.byte < rhs.byte;
    });

    if (entries.size() > BFGN_MAX_DICTIONARY_ENTRIES) {
        entries.resize(BFGN_MAX_DICTIONARY_ENTRIES);
    }

    for (std::size_t i = 0; i < entries.size(); ++i) {
        entries[i].code = static_cast<unsigned char>(i);
    }

    return entries;
}

}
