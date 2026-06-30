//
//  main.cpp
//  ByteForge
//
//  Created by Zeeshan Waheed on 14/05/2026.
//

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "Core/ByteReader.hpp"
#include "Core/ByteWriter.hpp"
#include "Analysis/RegexSequenceFinder.hpp"
#include "Analysis/PatternMapWriter.hpp"
#include "Compression/BFGCompressor.hpp"
#include "Compression/BFGDecompressor.hpp"
#include "Compression/BFGChunkedCompressor.hpp"
#include "Compression/BFGChunkedDecompressor.hpp"
#include "Nibble/NibbleCompressor.hpp"
#include "Nibble/NibbleChunkedCompressor.hpp"
#include "Nibble/NibbleChunkedDecompressor.hpp"
#include "Nibble/NibbleDecompressor.hpp"
#include "Validation/FileComparer.hpp"

namespace {

const std::string modelPath = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/model/synapse-qwen1.5b-q4_k_m.gguf";
const std::string samplePath = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/samples/synapse-qwen1.5b-first-5000.gguf";
const std::string outputDirectory = "/Users/zeeshanwaheed/Desktop/C++/ByteForge/ByteForge/Generated";

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

double elapsedMilliseconds(std::chrono::steady_clock::time_point start,
                           std::chrono::steady_clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::string formatElapsedTime(double milliseconds) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(3);

    if (milliseconds < 1000.0) {
        output << milliseconds << " ms";
        return output.str();
    }

    const double seconds = milliseconds / 1000.0;
    if (seconds < 60.0) {
        output << seconds << " s";
        return output.str();
    }

    const double minutes = seconds / 60.0;
    if (minutes < 60.0) {
        output << minutes << " min";
        return output.str();
    }

    const double hours = minutes / 60.0;
    if (hours < 24.0) {
        output << hours << " hr";
        return output.str();
    }

    const double days = hours / 24.0;
    output << days << " d";
    return output.str();
}

int readMenuChoice() {
    int choice = 0;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

bool runOldBFGTests() {
    const std::size_t oneMegabyte = 1024 * 1024;
    const std::vector<std::size_t> testSizes = {
        1 * oneMegabyte,
        10 * oneMegabyte,
        100 * oneMegabyte
    };

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
            return false;
        }

        if (!ByteForge::ByteWriter::writeBytes(sourceSlicePath, buffer)) {
            std::cerr << "Could not write source slice: " << sourceSlicePath << '\n';
            return false;
        }

        const std::vector<ByteForge::ByteRun> runs = ByteForge::RegexSequenceFinder::findRepeatedByteRuns(buffer);
        const ByteForge::ByteRun largestRun = ByteForge::RegexSequenceFinder::findLargestRun(buffer);
        const std::vector<ByteForge::BytePatternSummary> patterns = ByteForge::PatternMapWriter::summarizeRuns(runs);

        if (!ByteForge::PatternMapWriter::writeTextMap(patternMapPath, patterns)) {
            std::cerr << "Could not write pattern map: " << patternMapPath << '\n';
            return false;
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
        std::cout << std::setfill(' ');

        ByteForge::BFGCompressionResult compressionResult{};
        if (!ByteForge::BFGCompressor::compress(buffer, patterns, compressedPath, compressionResult)) {
            std::cerr << "Could not write compressed file: " << compressedPath << '\n';
            return false;
        }

        ByteForge::BFGDecompressionResult decompressionResult{};
        if (!ByteForge::BFGDecompressor::decompress(compressedPath, decompressedPath, decompressionResult)) {
            std::cerr << "Could not decompress file: " << compressedPath << '\n';
            return false;
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
            return false;
        }

        ByteForge::BFGChunkedDecompressionResult chunkedDecompressionResult{};
        if (!ByteForge::BFGChunkedDecompressor::decompress(chunkedCompressedPath,
                                                           chunkedDecompressedPath,
                                                           chunkedDecompressionResult)) {
            std::cerr << "Could not decompress chunked file: " << chunkedCompressedPath << '\n';
            return false;
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
            return false;
        }
    }

    return true;
}

bool runNibbleTest(const std::string& inputPath,
                   std::size_t bytesToRead,
                   const std::string& label) {
    const auto totalStart = std::chrono::steady_clock::now();
    std::vector<unsigned char> buffer;

    const bool didRead = bytesToRead == 0
        ? ByteForge::ByteReader::readAllBytes(inputPath, buffer)
        : ByteForge::ByteReader::readBytes(inputPath, bytesToRead, buffer);

    if (!didRead) {
        std::cerr << "Could not open input file: " << inputPath << '\n';
        return false;
    }

    const std::string sourcePath = outputDirectory + "/nibble-source-" + label + ".gguf";
    const std::string compressedPath = outputDirectory + "/nibble-chunked-" + label + ".bfgn";
    const std::string rebuiltPath = outputDirectory + "/nibble-rebuilt-" + label + ".gguf";

    if (!ByteForge::ByteWriter::writeBytes(sourcePath, buffer)) {
        std::cerr << "Could not write nibble source file: " << sourcePath << '\n';
        return false;
    }

    ByteForge::NibbleChunkedCompressionResult compressionResult{};
    const auto compressionStart = std::chrono::steady_clock::now();
    if (!ByteForge::NibbleChunkedCompressor::compress(buffer, compressedPath, compressionResult)) {
        std::cerr << "Could not write nibble compressed file: " << compressedPath << '\n';
        return false;
    }
    const auto compressionEnd = std::chrono::steady_clock::now();

    ByteForge::NibbleChunkedDecompressionResult decompressionResult{};
    const auto decompressionStart = std::chrono::steady_clock::now();
    if (!ByteForge::NibbleChunkedDecompressor::decompress(compressedPath, rebuiltPath, decompressionResult)) {
        std::cerr << "Could not decompress nibble file: " << compressedPath << '\n';
        return false;
    }
    const auto decompressionEnd = std::chrono::steady_clock::now();

    const ByteForge::FileComparisonResult comparison = ByteForge::FileComparer::compare(sourcePath, rebuiltPath);
    const auto totalEnd = std::chrono::steady_clock::now();

    std::cout << "\n=== Nibble Dictionary Test: " << label << " ===\n";
    std::cout << "Original bytes: " << compressionResult.originalSize << '\n';
    std::cout << "Chunks: " << compressionResult.chunkCount << '\n';
    std::cout << "Nibble chunks: " << compressionResult.nibbleChunkCount << '\n';
    std::cout << "Raw chunks: " << compressionResult.rawChunkCount << '\n';
    std::cout << "Nibble-encoded bytes: " << compressionResult.nibbleEncodedBytes << '\n';
    std::cout << "Raw bytes: " << compressionResult.rawBytes << '\n';
    std::cout << "Compressed file bytes: " << compressionResult.compressedFileSize << '\n';
    std::cout << "Saved: " << std::fixed << std::setprecision(2)
              << savedPercent(compressionResult.originalSize, compressionResult.compressedFileSize) << "%\n";
    std::cout << "Compression time: "
              << formatElapsedTime(elapsedMilliseconds(compressionStart, compressionEnd)) << '\n';
    std::cout << "Decompression time: "
              << formatElapsedTime(elapsedMilliseconds(decompressionStart, decompressionEnd)) << '\n';
    std::cout << "Total test time: "
              << formatElapsedTime(elapsedMilliseconds(totalStart, totalEnd)) << '\n';
    std::cout << "Rebuilt bytes: " << decompressionResult.decompressedSize << '\n';
    std::cout << "Rebuild matches source: " << (comparison.matches ? "yes" : "no") << '\n';
    std::cout << "Compressed file: " << compressedPath << '\n';
    std::cout << "Rebuilt file: " << rebuiltPath << '\n';

    return comparison.matches;
}

bool runNibbleMenu() {
    while (true) {
        std::cout << "\nNibble Dictionary Test\n\n";
        std::cout << "1. Test with default 5000-byte sample\n";
        std::cout << "2. Test with custom file\n";
        std::cout << "3. Back\n\n";
        std::cout << "Choose option: ";

        const int choice = readMenuChoice();

        if (choice == 1) {
            return runNibbleTest(samplePath, 0, "default-5000");
        }

        if (choice == 2) {
            std::string inputPath;
            std::size_t bytesToRead = 0;

            std::cout << "Enter input file path: ";
            std::getline(std::cin, inputPath);

            std::cout << "Enter number of bytes to read, or 0 for full file: ";
            std::cin >> bytesToRead;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            const std::string label = bytesToRead == 0 ? "custom-full" : "custom-" + sizeLabel(bytesToRead);
            return runNibbleTest(inputPath, bytesToRead, label);
        }

        if (choice == 3) {
            return true;
        }

        std::cout << "Invalid option.\n";
    }
}

}

int main(int argc, const char * argv[]) {
    std::cout << std::unitbuf;
    std::filesystem::create_directories(outputDirectory);

    while (true) {
        std::cout << "\nByteForge\n\n";
        std::cout << "1. Run old BFG repeated-byte tests\n";
        std::cout << "2. Run new nibble dictionary test\n";
        std::cout << "3. Exit\n\n";
        std::cout << "Choose option: ";

        const int choice = readMenuChoice();

        if (choice == 1) {
            if (!runOldBFGTests()) {
                return EXIT_FAILURE;
            }
            continue;
        }

        if (choice == 2) {
            if (!runNibbleMenu()) {
                return EXIT_FAILURE;
            }
            continue;
        }

        if (choice == 3) {
            return EXIT_SUCCESS;
        }

        std::cout << "Invalid option.\n";
    }
}
