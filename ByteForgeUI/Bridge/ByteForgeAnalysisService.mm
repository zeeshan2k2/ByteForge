// Exposes the ByteForge C++ research engine to SwiftUI through Objective-C++ wrappers.

#import "ByteForgeAnalysisService.h"

#include "../../ByteForge/Research/CompressionExperimentRunner.hpp"

@implementation ByteForgeDistributionMetrics

- (instancetype)initWithTotalBytes:(NSUInteger)totalBytes
                 distinctByteCount:(NSUInteger)distinctByteCount
                    zeroTo3Percent:(double)zeroTo3Percent
                   fourTo15Percent:(double)fourTo15Percent
                 sixteenTo63Percent:(double)sixteenTo63Percent
              sixtyFourTo127Percent:(double)sixtyFourTo127Percent
          oneTwentyEightTo255Percent:(double)oneTwentyEightTo255Percent
                      top4Coverage:(double)top4Coverage
                     top16Coverage:(double)top16Coverage
                     top64Coverage:(double)top64Coverage {
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _totalBytes = totalBytes;
    _distinctByteCount = distinctByteCount;
    _zeroTo3Percent = zeroTo3Percent;
    _fourTo15Percent = fourTo15Percent;
    _sixteenTo63Percent = sixteenTo63Percent;
    _sixtyFourTo127Percent = sixtyFourTo127Percent;
    _oneTwentyEightTo255Percent = oneTwentyEightTo255Percent;
    _top4Coverage = top4Coverage;
    _top16Coverage = top16Coverage;
    _top64Coverage = top64Coverage;

    return self;
}

@end

@implementation ByteForgeAnalysisResult

- (instancetype)initWithSuccess:(BOOL)success
                   errorMessage:(NSString *)errorMessage
                   originalSize:(NSUInteger)originalSize
                 compressedSize:(NSUInteger)compressedSize
                   savedPercent:(double)savedPercent
        compressionMilliseconds:(double)compressionMilliseconds
      decompressionMilliseconds:(double)decompressionMilliseconds
              totalMilliseconds:(double)totalMilliseconds
                     chunkCount:(NSUInteger)chunkCount
               nibbleChunkCount:(NSUInteger)nibbleChunkCount
                  rawChunkCount:(NSUInteger)rawChunkCount
                 rebuildMatches:(BOOL)rebuildMatches
                   distribution:(ByteForgeDistributionMetrics *)distribution {
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _success = success;
    _errorMessage = [errorMessage copy];
    _originalSize = originalSize;
    _compressedSize = compressedSize;
    _savedPercent = savedPercent;
    _compressionMilliseconds = compressionMilliseconds;
    _decompressionMilliseconds = decompressionMilliseconds;
    _totalMilliseconds = totalMilliseconds;
    _chunkCount = chunkCount;
    _nibbleChunkCount = nibbleChunkCount;
    _rawChunkCount = rawChunkCount;
    _rebuildMatches = rebuildMatches;
    _distribution = distribution;

    return self;
}

@end

@implementation ByteForgeAnalysisService

- (ByteForgeAnalysisResult *)runMethod:(ByteForgeCompressionMethod)method
                             inputPath:(NSString *)inputPath
                           bytesToRead:(NSUInteger)bytesToRead {
    const ByteForge::CompressionMethod cppMethod =
        method == ByteForgeCompressionMethodRepeatedByte
            ? ByteForge::CompressionMethod::RepeatedByte
            : ByteForge::CompressionMethod::AdaptiveNibble;

    const std::string outputDirectory =
        "/Users/zeeshanwaheed/Desktop/C++/ByteForge/ByteForge/Generated/UI";

    const ByteForge::CompressionExperimentResult result = ByteForge::CompressionExperimentRunner::run(
        cppMethod,
        std::string([inputPath UTF8String]),
        static_cast<std::size_t>(bytesToRead),
        outputDirectory
    );

    const auto& distribution = result.distribution;
    const double totalBytes = static_cast<double>(distribution.totalBytes);
    const double safeTotal = totalBytes == 0.0 ? 1.0 : totalBytes;

    ByteForgeDistributionMetrics *distributionMetrics = [[ByteForgeDistributionMetrics alloc]
        initWithTotalBytes:distribution.totalBytes
         distinctByteCount:distribution.distinctByteCount
            zeroTo3Percent:(static_cast<double>(distribution.zeroTo3Count) / safeTotal) * 100.0
           fourTo15Percent:(static_cast<double>(distribution.fourTo15Count) / safeTotal) * 100.0
         sixteenTo63Percent:(static_cast<double>(distribution.sixteenTo63Count) / safeTotal) * 100.0
      sixtyFourTo127Percent:(static_cast<double>(distribution.sixtyFourTo127Count) / safeTotal) * 100.0
  oneTwentyEightTo255Percent:(static_cast<double>(distribution.oneTwentyEightTo255Count) / safeTotal) * 100.0
              top4Coverage:distribution.top4Coverage
             top16Coverage:distribution.top16Coverage
             top64Coverage:distribution.top64Coverage];

    NSString *errorMessage = [NSString stringWithUTF8String:result.errorMessage.c_str()];
    return [[ByteForgeAnalysisResult alloc]
        initWithSuccess:result.success
           errorMessage:errorMessage
           originalSize:result.originalSize
         compressedSize:result.compressedSize
           savedPercent:result.savedPercent
compressionMilliseconds:result.compressionMilliseconds
decompressionMilliseconds:result.decompressionMilliseconds
      totalMilliseconds:result.totalMilliseconds
             chunkCount:result.chunkCount
       nibbleChunkCount:result.nibbleChunkCount
          rawChunkCount:result.rawChunkCount
         rebuildMatches:result.rebuildMatches
           distribution:distributionMetrics];
}

@end
