# CompressIt-FileCompression-BackupandSearchUtilityEngine

CompressIt is a C++ command-line utility that uses **Huffman Coding** for file compression and decompression, along with **hashing and content comparison** to identify duplicate files. It helps restore original file content, detect redundant files, and identify wasted storage space.

## Features

- **File Compression:** Compress files using Huffman Coding.
- **File Decompression:** Restore the original file content.
- **Duplicate Detection:** Identify identical files using hashing and content comparison.
- **Storage Analysis:** Calculate the storage space wasted by duplicate files.
- **Duplicate Removal:** Optionally delete extra copies while keeping one file.
- **Command-Line Interface:** Access all features through a simple menu.

## Technologies Used

- **Language:** C++
- **Data Structures:** Binary Trees, Priority Queue, Vectors, Hash Tables
- **Algorithms:** Huffman Coding and FNV-1a Hashing
- **Libraries:** STL, filesystem, fstream, queue, unordered_map, vector, algorithm

## Project Modules

1. **HuffmanCompressor:** Handles file compression and decompression.
2. **HuffmanNode:** Represents nodes used to build the Huffman tree.
3. **DuplicateDetector:** Finds and verifies duplicate files.
4. **Main Module:** Provides the command-line menu and connects the modules.

## How to Compile

Compile using a C++17-compatible compiler:

```bash
g++ -std=c++17 main.cpp HuffmanCompressor.cpp HuffmanNode.cpp DuplicateDetector.cpp -o CompressIt
```

## How to Run

On Linux or compatible terminals:

```bash
./CompressIt
```

On Windows, run:

```bash
CompressIt.exe
```

## Current Project Scope

The current implementation focuses on Huffman-based file compression, decompression, and hashing-based duplicate detection.

**Note:** Incremental backup and inverted-index text search were part of the original proposal but are not included in the current implementation scope.

## Future Improvements

- Improve error handling and validation.
- Measure compression ratios and execution time.
- Optimize performance for large files.
- Explore additional file-management features.

## Repository

GitHub: https://github.com/UDHAVJOSHI07/CompressIt-FileCompression-BackupandSearchUtilityEngine
