# ByteForge

ByteForge is a C++ experiment for compressing `.gguf` model files with custom binary formats.

The idea started while working with local SLMs, where even quantized models can still be 1GB+. ByteForge reads raw model bytes, tests compression strategies, writes custom compressed files, decompresses them back, and verifies that the rebuilt output matches the original byte-for-byte.

This is not a production compressor yet. It is a research/prototype project for testing whether `.gguf` files have useful byte-level structure that can be exploited.

## What It Does

- Reads raw bytes from a `.gguf` file.
- Tests repeated-byte compression.
- Tests chunked nibble-dictionary compression.
- Writes human-readable pattern maps for inspection.
- Writes custom binary formats such as `.bfg` and `.bfgn`.
- Decompresses compressed output back into `.gguf`.
- Compares rebuilt output against the source slice byte-for-byte.

Example idea:

```text
Original:
22 43 12 00 00 00

Compressed-style representation:
22 43 12 ff 03
```

In the `.bfg` implementation, `ff` is used as a marker byte. The byte after it tells the decompressor whether to emit a literal `ff` or expand a dictionary pattern.

## Tested Approaches

### 1. Repeated-Byte Compression

This was the first approach. It looks for repeated byte runs like:

```text
00 00 00 00 00 00 00
```

and stores them through a small dictionary/marker format.

Results:

```text
5000 bytes: ~43-46% saved
1MB:   ~33.8% saved
10MB:  ~16.0% saved
100MB: ~1.4% saved
```

This worked well on the start of the file because `.gguf` headers, metadata, and tokenizer sections contain many easy repeated patterns. The savings dropped hard once the test moved deeper into quantized model weights.

### 2. Nibble Dictionary Compression

The second approach maps common byte values into 4-bit nibble codes.

Example:

```text
nibble 0 -> byte 00
nibble 1 -> byte 10
nibble 2 -> byte ff
```

The first single-stream version worked well on the 5000-byte sample:

```text
Original bytes: 5000
Mapped bytes: 4283
Escaped bytes: 717
Compressed file bytes: 3254
Saved: 34.92%
Rebuild matches source: yes
```

But on the full model, too many bytes were outside the nibble dictionary, so the escape/literal path made the file bigger. That led to the safer `BFGN2` format.

`BFGN2` splits the file into chunks:

```text
good chunk -> nibble-compressed
bad chunk  -> stored raw
```

This avoids expanding the full model.

Results:

```text
5000 bytes:
compressed file bytes: 3954
saved: 20.92%
rebuild matches source: yes

1MB:
compressed file bytes: 1039261
saved: 0.89%
rebuild matches source: yes

full model:
original bytes: 986048096
compressed file bytes: 985588828
saved: 0.05%
rebuild matches source: yes
```

The full model result is small, but useful: it shows that the chunked nibble idea is lossless and safe, while also proving that most quantized model weight data is not nibble-friendly.

## File Formats

### BFG1 Repeated-Byte Stream

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

### BFG2 Repeated-Byte Chunks

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

### BFGN1 Nibble Dictionary Stream

Single nibble dictionary format:

```text
5 bytes   magic header: BFGN1
8 bytes   original size
1 byte    dictionary count
N bytes   dictionary byte values
8 bytes   compressed nibble count
N bytes   packed nibble stream
```

### BFGN2 Chunked Nibble/Raw Fallback

Chunked format:

```text
5 bytes   magic header: BFGN2
8 bytes   original size
4 bytes   chunk count

For each chunk:
1 byte    chunk type
4 bytes   original chunk size

If raw chunk:
N bytes   raw bytes

If nibble chunk:
1 byte    dictionary count
N bytes   dictionary byte values
2 bytes   packed stream size
N bytes   packed nibble stream
```

## Generated Files

`ByteForge/Generated/` is ignored by git. It contains generated benchmark artifacts such as:

```text
*.bfg
*.bfgn
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

The app currently opens a small console menu:

```text
1. Run old BFG repeated-byte tests
2. Run new nibble dictionary test
3. Exit
```

The nibble test can run against the default 5000-byte sample or a custom file path.

## Notes

So far, both tested approaches are lossless and rebuild the source bytes correctly. The main finding is that metadata-heavy regions compress well, but quantized tensor data is already dense and does not expose much simple byte-level structure.

Future directions:

- Multi-byte pattern dictionaries.
- Byte-pair frequency analysis.
- Tensor/block-aware compression.
- Streaming compression for full model files.
- Parallel chunk compression with `std::async`.
- Faster chunk scanning.
- Checksums for compressed file integrity.

## Status

Experimental. The compressor/decompressor loop works and rebuilds source slices byte-for-byte, but the compression approach is still early.
