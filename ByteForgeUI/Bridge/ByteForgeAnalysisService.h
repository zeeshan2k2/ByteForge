// Exposes the ByteForge C++ research engine to SwiftUI through Objective-C++ wrappers.

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, ByteForgeCompressionMethod) {
    ByteForgeCompressionMethodRepeatedByte = 0,
    ByteForgeCompressionMethodAdaptiveNibble = 1,
};

@interface ByteForgeDistributionMetrics : NSObject

@property (nonatomic, readonly) NSUInteger totalBytes;
@property (nonatomic, readonly) NSUInteger distinctByteCount;
@property (nonatomic, readonly) double zeroTo3Percent;
@property (nonatomic, readonly) double fourTo15Percent;
@property (nonatomic, readonly) double sixteenTo63Percent;
@property (nonatomic, readonly) double sixtyFourTo127Percent;
@property (nonatomic, readonly) double oneTwentyEightTo255Percent;
@property (nonatomic, readonly) double top4Coverage;
@property (nonatomic, readonly) double top16Coverage;
@property (nonatomic, readonly) double top64Coverage;

- (instancetype)initWithTotalBytes:(NSUInteger)totalBytes
                 distinctByteCount:(NSUInteger)distinctByteCount
                    zeroTo3Percent:(double)zeroTo3Percent
                   fourTo15Percent:(double)fourTo15Percent
                 sixteenTo63Percent:(double)sixteenTo63Percent
              sixtyFourTo127Percent:(double)sixtyFourTo127Percent
          oneTwentyEightTo255Percent:(double)oneTwentyEightTo255Percent
                      top4Coverage:(double)top4Coverage
                     top16Coverage:(double)top16Coverage
                     top64Coverage:(double)top64Coverage;

@end

@interface ByteForgeAnalysisResult : NSObject

@property (nonatomic, readonly) BOOL success;
@property (nonatomic, copy, readonly) NSString *errorMessage;
@property (nonatomic, readonly) NSUInteger originalSize;
@property (nonatomic, readonly) NSUInteger compressedSize;
@property (nonatomic, readonly) double savedPercent;
@property (nonatomic, readonly) double compressionMilliseconds;
@property (nonatomic, readonly) double decompressionMilliseconds;
@property (nonatomic, readonly) double totalMilliseconds;
@property (nonatomic, readonly) NSUInteger chunkCount;
@property (nonatomic, readonly) NSUInteger nibbleChunkCount;
@property (nonatomic, readonly) NSUInteger rawChunkCount;
@property (nonatomic, readonly) BOOL rebuildMatches;
@property (nonatomic, strong, readonly) ByteForgeDistributionMetrics *distribution;

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
                   distribution:(ByteForgeDistributionMetrics *)distribution;

@end

@interface ByteForgeAnalysisService : NSObject

- (ByteForgeAnalysisResult *)runMethod:(ByteForgeCompressionMethod)method
                             inputPath:(NSString *)inputPath
                           bytesToRead:(NSUInteger)bytesToRead;

@end

NS_ASSUME_NONNULL_END
