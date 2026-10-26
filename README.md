# Huffman File Compressor (C++17)

A robust, high-performance C++17 implementation of **lossless data compression and decompression** using **Huffman Coding**. Designed for Operating Systems / Systems Programming coursework and general-purpose file compression.

---

## 🌟 Features

- **True Bit-Level Compression**: Implements bit-level stream reading and writing (`BitWriter` and `BitReader`) to pack variable-length prefix codes compactly instead of wasting bytes.
- **Flexible Path Input**:
  - **Single `.txt` File**: Directly compresses the file and generates `<filename>.huf` in the **exact same directory**.
  - **Directory containing `.txt` files**: Scans the directory, finds all `.txt` files, and compresses each file into its respective folder.
  - **Interactive Mode**: If run without arguments, interactively prompts for a path.
- **Self-Contained Compressed Files**: Serializes the Huffman tree structure into the header so the file can be decompressed anywhere without external lookup tables.
- **Round-Trip Decompression**: Full decompression capability with automatic validation to ensure byte-for-byte fidelity with the original file.
- **Detailed Compression Metrics**: Reports original file size, compressed size, compression ratio, space savings percentage, and execution duration.
- **Edge-Case Resilience**: Seamlessly handles empty files, single-character files, binary byte values (0–255), and large files with chunked 64KB I/O buffering.

---

## 📂 Project Structure

```
project/
├── include/
│   ├── BitStream.hpp          # BitWriter and BitReader for bit-level I/O
│   ├── HuffmanNode.hpp        # Huffman tree node definition & min-heap comparator
│   ├── HuffmanTree.hpp        # Tree construction, prefix code generation & serialization
│   ├── HuffmanCompressor.hpp  # Compression & decompression orchestrator
│   └── FileUtils.hpp          # Path resolution, directory scanning, formatting
├── src/
│   ├── BitStream.cpp          # Bit packing and stream buffer logic
│   ├── HuffmanTree.cpp        # Algorithm implementation (priority queue & traversal)
│   ├── HuffmanCompressor.cpp  # Two-pass compression & lossless reconstruction
│   ├── FileUtils.cpp          # Filesystem utilities (C++17 std::filesystem)
│   └── main.cpp               # CLI interface & interactive path handler
├── tests/
│   └── test_runner.cpp        # Unit and integration test suite
├── examples/
│   └── sample.txt             # Sample text file for immediate testing
├── Makefile                   # Build configuration (build, test, clean)
├── CMakeLists.txt             # Alternative CMake build configuration
└── README.md                  # Project documentation
```

---

## ⚙️ Building the Project

### Requirements
- **GCC / G++ 9+** or **Clang 9+** (C++17 standard support)
- **Make**

### Build Commands

```bash
# Build the main executable (bin/huffman)
make all

# Build and run the test suite
make test

# Clean build artifacts
make clean
```

---

## 🚀 Usage

### 1. Compress a `.txt` File
Pass the path to any `.txt` file:
```bash
./bin/huffman /path/to/my_file.txt
```
> The compressed file `my_file.txt.huf` is placed in the **same directory** (`/path/to/`).

### 2. Compress a Directory Containing `.txt` Files
Pass the directory path:
```bash
./bin/huffman /path/to/my_folder
```
This scans `/path/to/my_folder`, finds all `.txt` files, and generates `.huf` files next to each uncompressed file.

To recursively scan subdirectories, add `-r`:
```bash
./bin/huffman -r /path/to/my_folder
```

### 3. Interactive Mode
Simply run the program with no arguments:
```bash
./bin/huffman
```
You will be prompted:
```text
Enter path to a .txt file or directory containing .txt files: /home/user/notes.txt
```

### 4. Decompress a File
To restore a compressed file back to its original text:
```bash
./bin/huffman -d /path/to/my_file.txt.huf
```
Or simply pass the `.huf` file directly:
```bash
./bin/huffman /path/to/my_file.txt.huf
```
> Restores the file to the same directory (as `my_file.txt` or `my_file_decompressed.txt` if original still exists).

---

## 📊 File Format Specification (`.huf`)

The output binary `.huf` file is structured as follows:

| Field | Size | Description |
| :--- | :--- | :--- |
| **Magic Header** | 4 Bytes | ASCII signature: `H`, `U`, `F`, `0x01` |
| **Original Size** | 8 Bytes (`uint64_t`) | Total uncompressed file size in bytes |
| **Distinct Symbols** | 2 Bytes (`uint16_t`) | Number of distinct byte symbols (0–256) |
| **Serialized Tree** | Variable bits | Compact pre-order tree traversal (`0` for internal node, `1` followed by 8-bit byte for leaf) |
| **Encoded Payload** | Variable bits | Bitstream of variable-length Huffman codes |
| **Padding** | 0–7 bits | Padded to the nearest byte boundary |

Because the uncompressed size is recorded in the header, any trailing padding bits in the final byte are safely ignored during decoding.

---

## 🧪 Testing and Verification

Run the automated test suite:
```bash
make test
```

The test runner validates:
1. **Empty File**: Correct handling of 0-byte input files.
2. **Single Character Repeated**: Correct handling of degenerate single-node trees (e.g., 500 'Z' characters).
3. **Standard English Text**: Real text compression and ratio calculation.
4. **All 256 Byte Values (Binary Purity)**: Complete byte spectrum round-trip test.
5. **Directory Batch Mode**: Verifies directory scanning and output co-location.
6. **Byte-for-byte verification**: Verifies that original file == decompressed file via direct memory buffer comparison (`memcmp`).

