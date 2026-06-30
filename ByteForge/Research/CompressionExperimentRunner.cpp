// Runs ByteForge compression experiments and returns UI-friendly result structs.

#include "CompressionExperimentRunner.hpp"

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

#include "../Analysis/PatternMapWriter.hpp"
#include "../Analysis/RegexSequenceFinder.hpp"
#include "../Compression/BFGChunkedCompressor.hpp"
#include "../Compression/BFGChunkedDecompressor.hpp"
#include "../Core/ByteReader.hpp"
#include "../Core/ByteWriter.hpp"
#include "../Nibble/NibbleChunkedCompressor.hpp"
#include "../Nibble/NibbleChunkedDecompressor.hpp"
#include "../Validation/FileComparer.hpp"

namespace ByteForge {
namespace {

double elapsedMilliseconds(std::chrono::steady_clock::time_point start,
                           std::chrono::steady_clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

double savedPercent(std::size_t originalBytes, std::size_t compressedBytes) {
    if (originalBytes == 0) {
        return 0.0;
    }

    return (1.0 - (static_cast<double>(compressedBytes) / static_cast<double>(originalBytes))) * 100.0;
}

std::string methodPrefix(CompressionMethod method) {
    switch (method) {
        case CompressionMethod::RepeatedByte:
            return "ui-repeated";
        case CompressionMethod::AdaptiveNibble:
            return "ui-adaptive-nibble";
    }

    return "ui";
}

CompressionExperimentResult makeFailure(const std::string& message) {
    return CompressionExperimentResult{
        false,
        message,
        0,
        0,
        0.0,
        0.0,
        0.0,
        0.0,
        0,
        0,
        0,
        false,
        ByteDistributionMetrics{}
    };
}

}

CompressionExperimentResult CompressionExperimentRunner::run(CompressionMethod method,
                                                             const std::string& inputPath,
                                                             std::size_t bytesToRead,
                                                             const std::string& outputDirectory) {
    const auto totalStart = std::chrono::steady_clock::now();
    std::vector<unsigned char> bytes;

    const bool readSucceeded = bytesToRead == 0
        ? ByteReader::readAllBytes(inputPath, bytes)
        : ByteReader::readBytes(inputPath, bytesToRead, bytes);

    if (!readSucceeded) {
        return makeFailure("Could not read input file.");
    }

    std::filesystem::create_directories(outputDirectory);

    const ByteDistributionMetrics distribution = ByteDistributionAnalyzer::analyze(bytes);
    const std::string prefix = methodPrefix(method) + "-" + std::to_string(bytes.size());
    const std::string sourcePath = outputDirectory + "/" + prefix + "-source.gguf";

    if (!ByteWriter::writeBytes(sourcePath, bytes)) {
        return makeFailure("Could not write source snapshot.");
    }

    if (method == CompressionMethod::RepeatedByte) {
        const std::string compressedPath = outputDirectory + "/" + prefix + ".bfg";
        const std::string rebuiltPath = outputDirectory + "/" + prefix + "-rebuilt.gguf";

        const std::vector<ByteRun> runs = RegexSequenceFinder::findRepeatedByteRuns(bytes);
        const std::vector<BytePatternSummary> patterns = PatternMapWriter::summarizeRuns(runs);

        const auto compressionStart = std::chrono::steady_clock::now();
        BFGChunkedCompressionResult compressionResult{};
        if (!BFGChunkedCompressor::compress(bytes, 5, compressedPath, compressionResult)) {
            return makeFailure("Repeated-byte compression failed.");
        }
        const auto compressionEnd = std::chrono::steady_clock::now();

        const auto decompressionStart = std::chrono::steady_clock::now();
        BFGChunkedDecompressionResult decompressionResult{};
        if (!BFGChunkedDecompressor::decompress(compressedPath, rebuiltPath, decompressionResult)) {
            return makeFailure("Repeated-byte decompression failed.");
        }
        const auto decompressionEnd = std::chrono::steady_clock::now();

        const FileComparisonResult comparison = FileComparer::compare(sourcePath, rebuiltPath);
        const auto totalEnd = std::chrono::steady_clock::now();

        return CompressionExperimentResult{
            comparison.matches,
            comparison.matches ? "" : "Repeated-byte rebuilt output did not match source.",
            bytes.size(),
            compressionResult.compressedFileSize,
            savedPercent(bytes.size(), compressionResult.compressedFileSize),
            elapsedMilliseconds(compressionStart, compressionEnd),
            elapsedMilliseconds(decompressionStart, decompressionEnd),
            elapsedMilliseconds(totalStart, totalEnd),
            compressionResult.chunkCount,
            0,
            0,
            comparison.matches,
            distribution
        };
    }

    const std::string compressedPath = outputDirectory + "/" + prefix + ".bfgn";
    const std::string rebuiltPath = outputDirectory + "/" + prefix + "-rebuilt.gguf";

    const auto compressionStart = std::chrono::steady_clock::now();
    NibbleChunkedCompressionResult compressionResult{};
    if (!NibbleChunkedCompressor::compress(bytes, compressedPath, compressionResult)) {
        return makeFailure("Adaptive nibble compression failed.");
    }
    const auto compressionEnd = std::chrono::steady_clock::now();

    const auto decompressionStart = std::chrono::steady_clock::now();
    NibbleChunkedDecompressionResult decompressionResult{};
    if (!NibbleChunkedDecompressor::decompress(compressedPath, rebuiltPath, decompressionResult)) {
        return makeFailure("Adaptive nibble decompression failed.");
    }
    const auto decompressionEnd = std::chrono::steady_clock::now();

    const FileComparisonResult comparison = FileComparer::compare(sourcePath, rebuiltPath);
    const auto totalEnd = std::chrono::steady_clock::now();

    return CompressionExperimentResult{
        comparison.matches,
        comparison.matches ? "" : "Adaptive nibble rebuilt output did not match source.",
        bytes.size(),
        compressionResult.compressedFileSize,
        savedPercent(bytes.size(), compressionResult.compressedFileSize),
        elapsedMilliseconds(compressionStart, compressionEnd),
        elapsedMilliseconds(decompressionStart, decompressionEnd),
        elapsedMilliseconds(totalStart, totalEnd),
        compressionResult.chunkCount,
        compressionResult.nibbleChunkCount,
        compressionResult.rawChunkCount,
        comparison.matches,
        distribution
    };
}

}
