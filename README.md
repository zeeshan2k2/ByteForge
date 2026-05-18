# ByteForge

ByteForge is a C++ experiment for compressing `.gguf` model files with a custom binary format.

The idea started while working with local SLMs, where even quantized models can still be 1GB+. ByteForge reads raw model bytes, finds repeated byte patterns, writes a custom `.bfg` compressed file, decompresses it back, and verifies that the rebuilt file matches the original byte-for-byte.

This is not a production compressor yet. It is a research/prototype project for testing whether `.gguf` files have useful byte-level structure that can be exploited.

## What It Does

- Reads raw bytes from a `.gguf` file.
- Finds repeated byte runs such as `00 00 00 00`.
- Writes a human-readable pattern map for inspection.
- Compresses bytes into a custom `.bfg` binary format.
- Supports two `.bfg` formats:
  - `BFG1`: single compressed stream.
  - `BFG2`: chunked compressed stream.
- Decompresses `.bfg` back into `.gguf`.
- Compares rebuilt output against the source slice byte-for-byte.

Example idea:

```text
Original:
22 43 12 00 00 00

Compressed-style representation:
22 43 12 ff 03
```

In the current `.bfg` implementation, `ff` is used as a marker byte. The byte after it tells the decompressor whether to emit a literal `ff` or expand a dictionary pattern.

## Current Results

The first 5000-byte sample compressed extremely well because it mostly contains `.gguf` header/metadata:

```text
5000 bytes -> about 2730-2850 bytes
~43-46% reduction
```

After testing larger slices of the real model, the compression ratio dropped:

```text
1MB:   ~33.8% saved
10MB:  ~16.0% saved
100MB: ~1.4% saved
```

This makes sense. The early part of a `.gguf` file contains metadata and tokenizer data, which has many repeated patterns. Deeper into the file, the quantized model weights are already densely packed, so simple repeated-byte compression has much less to work with.

## File Formats

### BFG1

Single-stream format:

```text
4 bytes   magic header: BFG1
8 bytes   original size
2 bytes   dictionary count

For each dictionary entry:
1 byte    pattern id
1 byte    pattern length
N bytes   pattern bytes

8 bytes   compressed stream size
N bytes   compressed stream
```

### BFG2

Chunked format:

```text
4 bytes   magic header: BFG2
8 bytes   original full size
2 bytes   chunk count

For each chunk:
8 bytes   original chunk size
2 bytes   dictionary count

For each dictionary entry:
1 byte    pattern id
1 byte    pattern length
N bytes   pattern bytes

8 bytes   compressed stream size
N bytes   compressed stream
```

## Generated Files

`ByteForge/Generated/` is ignored by git. It contains generated benchmark artifacts such as:

```text
*.bfg
*.bfgmap
rebuilt-*.gguf
source-*.gguf
```

The real model file is also ignored:

```text
model/*.gguf
```

## Running

Open the project in Xcode:

```text
ByteForge.xcodeproj
```

Or build from terminal:

```bash
xcodebuild -project ByteForge.xcodeproj -scheme ByteForge -configuration Debug build
```

The current `main.cpp` expects the model at:

```text
/Users/zeeshanwaheed/Desktop/C++/ByteForge/model/synapse-qwen1.5b-q4_k_m.gguf
```

The benchmark sizes are defined in `main.cpp`:

```cpp
const std::vector<std::size_t> testSizes = {
    1 * oneMegabyte,
    10 * oneMegabyte,
    100 * oneMegabyte
};
```

You can add or remove sizes from that array to test different slices.

## Notes

This project currently uses a simple repeated-byte dictionary scheme. It works well on metadata-heavy chunks, but not as well on quantized tensor data.

Future directions:

- Multi-byte pattern dictionaries.
- Byte-pair frequency analysis.
- Tensor/block-aware compression.
- Streaming compression for full model files.
- Parallel chunk compression with `std::async`.
- Checksums for compressed file integrity.

## Status

Experimental. The compressor/decompressor loop works and rebuilds source slices byte-for-byte, but the compression approach is still early.

