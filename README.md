# ByteForge

ByteForge is an experimental C++ project exploring custom compression techniques for `.gguf` model files.

The project began while working with local Small Language Models (SLMs), where even quantized models can remain hundreds of megabytes or multiple gigabytes in size. ByteForge investigates whether `.gguf` files contain useful byte-level patterns that can be exploited through custom compression formats and encoding strategies.

The project is currently focused on research, experimentation, benchmarking, and understanding the structure of quantized model files rather than building a production-ready compressor.

---

## Overview

ByteForge reads raw model bytes, applies custom compression strategies, rebuilds the original file through decompression, and verifies that the reconstructed output matches the source byte-for-byte.

Current experiments include:

- Repeated-byte compression
- Dictionary-based compression
- Nibble encoding
- Chunk-based compression
- Custom binary file formats
- Compression benchmarking
- Lossless validation

---

## Notes

<p>
Detailed research notes, benchmarks, findings, and implementation experiments can be found here:<br>
<a href="https://secretive-fascinator-c6c.notion.site/Project-Name-ByteForge-In-progress-GitHub-357d7fcea352808ea5aed6d20c8e21be?source=copy_link">
ByteForge Research Notes (Notion)
</a>
</p>

---

## Tech Stack

- **Language:** C++
- **Build System:** Xcode
- **Focus:** Binary Compression Research
- **Target Format:** GGUF

---

## Key Areas Explored

- Binary file analysis
- Custom compression formats
- Dictionary encoding
- Nibble-based representations
- Chunk-based compression strategies
- Lossless reconstruction
- Model file internals

---

## Project Structure

```text
ByteForge/
├── Compressor/
├── Decompressor/
├── Formats/
├── Benchmarks/
├── Utilities/
├── Generated/        (ignored)
├── model/            (ignored)
└── ByteForge.xcodeproj
```

---

## Current Status

ByteForge is an active research project. The compressor/decompressor pipeline is functional and capable of rebuilding source data correctly, while compression strategies continue to be evaluated and refined.
