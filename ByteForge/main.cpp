//
//  main.cpp
//  ByteForge
//
//  Created by Zeeshan Waheed on 14/05/2026.
//

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "Compression/RegexSequenceFinder.hpp"

int main(int argc, const char * argv[]) {
    const std::string modelPath = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/samples/synapse-qwen1.5b-first-5000.gguf";

    std::ifstream modelFile(modelPath, std::ios::binary);

    if (!modelFile) {
        std::cerr << "Could not open GGUF file: " << modelPath << '\n';
        return EXIT_FAILURE;
    }

    const int bytesToRead = 100;
    const int bytesPerChunk = 16;
    std::vector<unsigned char> buffer(bytesToRead);

    modelFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
    const std::streamsize bytesRead = modelFile.gcount();
    buffer.resize(static_cast<std::size_t>(bytesRead));

    for (std::size_t chunkStart = 0; chunkStart < buffer.size(); chunkStart += bytesPerChunk) {
        const std::size_t chunkEnd = std::min(chunkStart + bytesPerChunk, buffer.size());
        for (std::size_t i = chunkStart; i < chunkEnd; ++i) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buffer[i]);
            if (i + 1 < chunkEnd) {
                std::cout << ' ';
            }
        }
        std::cout << " | memory: " << std::dec << (chunkEnd - chunkStart) << " bytes\n";
    }

    const std::vector<ByteForge::ByteRun> runs = ByteForge::RegexSequenceFinder::findRepeatedByteRuns(buffer);
    const ByteForge::ByteRun largestRun = ByteForge::RegexSequenceFinder::findLargestRun(buffer);

    std::cout << "\nRepeated byte runs:\n";
    for (const ByteForge::ByteRun& run : runs) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(run.byte)
                  << " x" << std::dec << run.length
                  << " at byte " << run.startIndex << '\n';
    }

    if (largestRun.length > 0) {
        std::cout << "\nLargest run: "
                  << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(largestRun.byte)
                  << " x" << std::dec << largestRun.length
                  << " at byte " << largestRun.startIndex << '\n';
    } else {
        std::cout << "\nLargest run: none\n";
    }

    std::cout << "Total memory printed: " << buffer.size() << " bytes\n";

    return EXIT_SUCCESS;
}
