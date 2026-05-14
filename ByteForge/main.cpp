//
//  main.cpp
//  ByteForge
//
//  Created by Zeeshan Waheed on 14/05/2026.
//

#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "Core/ByteReader.hpp"
#include "Core/ByteWriter.hpp"
#include "Analysis/RegexSequenceFinder.hpp"
#include "Analysis/PatternMapWriter.hpp"
#include "Compression/BFGCompressor.hpp"
#include "Compression/BFGDecompressor.hpp"
#include "Compression/BFGChunkedCompressor.hpp"
#include "Compression/BFGChunkedDecompressor.hpp"
#include "Validation/FileComparer.hpp"

namespace {

std::string sizeLabel(std::size_t bytes) {
    const std::size_t oneMegabyte = 1024 * 1024;

    if (bytes % oneMegabyte == 0) {
        return std::to_string(bytes / oneMegabyte) + "mb";
    }

    return std::to_string(bytes) + "bytes";
}

double savedPercent(std::size_t originalBytes, std::size_t compressedBytes) {
    if (originalBytes == 0) {
        return 0.0;
    }

    return (1.0 - (static_cast<double>(compressedBytes) / static_cast<double>(originalBytes))) * 100.0;
}

}

int main(int argc, const char * argv[]) {
    std::cout << std::unitbuf;

    const std::string modelPath = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/model/synapse-qwen1.5b-q4_k_m.gguf";
    const std::string outputDirectory = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/ByteForge/Generated";
    const std::size_t oneMegabyte = 1024 * 1024;
    const std::vector<std::size_t> testSizes = {
        1 * oneMegabyte,
        10 * oneMegabyte,
        100 * oneMegabyte
    };

    std::filesystem::create_directories(outputDirectory);

    for (const std::size_t testSize : testSizes) {
        const std::string label = sizeLabel(testSize);
        const std::string sourceSlicePath = outputDirectory + "/source-" + label + ".gguf";
        const std::string patternMapPath = outputDirectory + "/patterns-" + label + ".bfgmap";
        const std::string compressedPath = outputDirectory + "/sample-" + label + ".bfg";
        const std::string decompressedPath = outputDirectory + "/rebuilt-" + label + ".gguf";
        const std::string chunkedCompressedPath = outputDirectory + "/sample-" + label + "-chunked.bfg";
        const std::string chunkedDecompressedPath = outputDirectory + "/rebuilt-" + label + "-chunked.gguf";

        std::cout << "\n=== Testing " << label << " ===\n";

        std::vector<unsigned char> buffer;
        if (!ByteForge::ByteReader::readBytes(modelPath, testSize, buffer)) {
            std::cerr << "Could not open GGUF file: " << modelPath << '\n';
            return EXIT_FAILURE;
        }

        if (!ByteForge::ByteWriter::writeBytes(sourceSlicePath, buffer)) {
            std::cerr << "Could not write source slice: " << sourceSlicePath << '\n';
            return EXIT_FAILURE;
        }

        const std::vector<ByteForge::ByteRun> runs = ByteForge::RegexSequenceFinder::findRepeatedByteRuns(buffer);
        const ByteForge::ByteRun largestRun = ByteForge::RegexSequenceFinder::findLargestRun(buffer);
        const std::vector<ByteForge::BytePatternSummary> patterns = ByteForge::PatternMapWriter::summarizeRuns(runs);

        if (!ByteForge::PatternMapWriter::writeTextMap(patternMapPath, patterns)) {
            std::cerr << "Could not write pattern map: " << patternMapPath << '\n';
            return EXIT_FAILURE;
        }

        std::cout << "Read bytes: " << buffer.size() << '\n';
        std::cout << "Pattern entries: " << patterns.size() << '\n';
        if (largestRun.length > 0) {
            std::cout << "Largest run: "
                      << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(largestRun.byte)
                      << " x" << std::dec << largestRun.length
                      << " at byte " << largestRun.startIndex << '\n';
        } else {
            std::cout << "Largest run: none\n";
        }

        ByteForge::BFGCompressionResult compressionResult{};
        if (!ByteForge::BFGCompressor::compress(buffer, patterns, compressedPath, compressionResult)) {
            std::cerr << "Could not write compressed file: " << compressedPath << '\n';
            return EXIT_FAILURE;
        }

        ByteForge::BFGDecompressionResult decompressionResult{};
        if (!ByteForge::BFGDecompressor::decompress(compressedPath, decompressedPath, decompressionResult)) {
            std::cerr << "Could not decompress file: " << compressedPath << '\n';
            return EXIT_FAILURE;
        }

        const ByteForge::FileComparisonResult singleStreamComparison = ByteForge::FileComparer::compare(sourceSlicePath, decompressedPath);
        std::cout << "BFG1 dictionary entries: " << compressionResult.dictionaryCount << '\n';
        std::cout << "BFG1 compressed stream bytes: " << compressionResult.compressedStreamSize << '\n';
        std::cout << "BFG1 compressed file bytes: " << compressionResult.compressedFileSize << '\n';
        std::cout << "BFG1 saved: " << std::fixed << std::setprecision(2)
                  << savedPercent(buffer.size(), compressionResult.compressedFileSize) << "%\n";
        std::cout << "BFG1 rebuild matches source slice: "
                  << (singleStreamComparison.matches ? "yes" : "no") << '\n';

        ByteForge::BFGChunkedCompressionResult chunkedCompressionResult{};
        if (!ByteForge::BFGChunkedCompressor::compress(buffer, 5, chunkedCompressedPath, chunkedCompressionResult)) {
            std::cerr << "Could not write chunked compressed file: " << chunkedCompressedPath << '\n';
            return EXIT_FAILURE;
        }

        ByteForge::BFGChunkedDecompressionResult chunkedDecompressionResult{};
        if (!ByteForge::BFGChunkedDecompressor::decompress(chunkedCompressedPath,
                                                           chunkedDecompressedPath,
                                                           chunkedDecompressionResult)) {
            std::cerr << "Could not decompress chunked file: " << chunkedCompressedPath << '\n';
            return EXIT_FAILURE;
        }

        const ByteForge::FileComparisonResult chunkedComparison = ByteForge::FileComparer::compare(sourceSlicePath, chunkedDecompressedPath);
        std::cout << "BFG2 chunk count: " << chunkedCompressionResult.chunkCount << '\n';
        std::cout << "BFG2 dictionary entries: " << chunkedCompressionResult.totalDictionaryEntries << '\n';
        std::cout << "BFG2 compressed stream bytes: " << chunkedCompressionResult.totalCompressedStreamSize << '\n';
        std::cout << "BFG2 compressed file bytes: " << chunkedCompressionResult.compressedFileSize << '\n';
        std::cout << "BFG2 saved: " << std::fixed << std::setprecision(2)
                  << savedPercent(buffer.size(), chunkedCompressionResult.compressedFileSize) << "%\n";
        std::cout << "BFG2 rebuild matches source slice: "
                  << (chunkedComparison.matches ? "yes" : "no") << '\n';

        if (!singleStreamComparison.matches || !chunkedComparison.matches) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
