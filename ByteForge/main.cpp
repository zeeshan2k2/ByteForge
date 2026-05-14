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
#include <filesystem>

#include "Analysis/RegexSequenceFinder.hpp"
#include "Analysis/PatternMapWriter.hpp"
#include "Compression/BFGCompressor.hpp"
#include "Compression/BFGDecompressor.hpp"

int main(int argc, const char * argv[]) {
    const std::string modelPath = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/samples/synapse-qwen1.5b-first-5000.gguf";
    const std::string outputDirectory = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/ByteForge/Generated";
    const std::string patternMapPath = outputDirectory + "/patterns.bfgmap";
    const std::string compressedPath = outputDirectory + "/sample.bfg";
    const std::string decompressedPath = outputDirectory + "/rebuilt.gguf";

    std::ifstream modelFile(modelPath, std::ios::binary);

    if (!modelFile) {
        std::cerr << "Could not open GGUF file: " << modelPath << '\n';
        return EXIT_FAILURE;
    }

    const int bytesToRead = 5000;
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
    const std::vector<ByteForge::BytePatternSummary> patterns = ByteForge::PatternMapWriter::summarizeRuns(runs);

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

    std::filesystem::create_directories(outputDirectory);
    if (!ByteForge::PatternMapWriter::writeTextMap(patternMapPath, patterns)) {
        std::cerr << "Could not write pattern map: " << patternMapPath << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Pattern map written: " << patternMapPath << '\n';
    std::cout << "Pattern entries: " << patterns.size() << '\n';

    ByteForge::BFGCompressionResult compressionResult{};
    if (!ByteForge::BFGCompressor::compress(buffer, patterns, compressedPath, compressionResult)) {
        std::cerr << "Could not write compressed file: " << compressedPath << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Compressed file written: " << compressedPath << '\n';
    std::cout << "Dictionary entries used: " << compressionResult.dictionaryCount << '\n';
    std::cout << "Original bytes: " << compressionResult.originalSize << '\n';
    std::cout << "Compressed stream bytes: " << compressionResult.compressedStreamSize << '\n';
    std::cout << "Compressed file bytes: " << compressionResult.compressedFileSize << '\n';

    ByteForge::BFGDecompressionResult decompressionResult{};
    if (!ByteForge::BFGDecompressor::decompress(compressedPath, decompressedPath, decompressionResult)) {
        std::cerr << "Could not decompress file: " << compressedPath << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "Decompressed file written: " << decompressedPath << '\n';
    std::cout << "Decompressed bytes: " << decompressionResult.decompressedSize << '\n';

    return EXIT_SUCCESS;
}
