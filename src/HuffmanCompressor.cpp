#include "HuffmanCompressor.hpp"
#include "HuffmanTree.hpp"
#include "BitStream.hpp"
#include "FileUtils.hpp"

#include <fstream>
#include <stdexcept>
#include <array>
#include <vector>
#include <chrono>

constexpr uint8_t HuffmanCompressor::MAGIC[4];

namespace {
    // Helper to write integers in little-endian format
    template <typename T>
    void writeBinary(std::ostream& os, T value) {
        for (size_t i = 0; i < sizeof(T); ++i) {
            os.put(static_cast<char>((value >> (i * 8)) & 0xFF));
        }
    }

    // Helper to read integers in little-endian format
    template <typename T>
    bool readBinary(std::istream& is, T& value) {
        value = 0;
        for (size_t i = 0; i < sizeof(T); ++i) {
            int byteRead = is.get();
            if (byteRead == EOF) return false;
            value |= (static_cast<T>(static_cast<uint8_t>(byteRead)) << (i * 8));
        }
        return true;
    }
}

CompressionStats HuffmanCompressor::compress(const std::string& inputPath, const std::string& outputPath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    CompressionStats stats;
    stats.inputPath = inputPath;
    stats.outputPath = outputPath.empty() ? FileUtils::getCompressedPath(inputPath) : outputPath;

    // Check if input file exists
    if (!FileUtils::exists(inputPath) || !FileUtils::isRegularFile(inputPath)) {
        throw std::runtime_error("Input file does not exist or is not a regular file: " + inputPath);
    }

    // Pass 1: Calculate byte frequencies
    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open input file for reading: " + inputPath);
    }

    std::array<uint64_t, 256> frequencies{};
    std::vector<char> readBuffer(65536);
    uint64_t totalBytes = 0;

    while (inFile.read(readBuffer.data(), readBuffer.size()) || inFile.gcount() > 0) {
        std::streamsize bytesRead = inFile.gcount();
        totalBytes += bytesRead;
        for (std::streamsize i = 0; i < bytesRead; ++i) {
            frequencies[static_cast<uint8_t>(readBuffer[i])]++;
        }
    }

    stats.originalSize = totalBytes;

    uint16_t distinctCount = 0;
    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            distinctCount++;
        }
    }
    stats.distinctSymbols = distinctCount;

    // Build Huffman tree & generate codes
    HuffmanTree tree;
    tree.buildFromFrequencies(frequencies);
    auto codes = tree.generateCodes();

    // Open output file
    std::ofstream outFile(stats.outputPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open output file for writing: " + stats.outputPath);
    }

    // Write file header: Magic (4B), OriginalSize (8B), DistinctSymbols (2B)
    outFile.write(reinterpret_cast<const char*>(MAGIC), 4);
    writeBinary<uint64_t>(outFile, stats.originalSize);
    writeBinary<uint16_t>(outFile, stats.distinctSymbols);

    // If file is not empty, serialize tree and compress data
    if (stats.originalSize > 0) {
        BitWriter bitWriter(outFile);
        tree.serialize(bitWriter);

        // Pass 2: Encode payload
        inFile.clear();
        inFile.seekg(0, std::ios::beg);

        while (inFile.read(readBuffer.data(), readBuffer.size()) || inFile.gcount() > 0) {
            std::streamsize bytesRead = inFile.gcount();
            for (std::streamsize i = 0; i < bytesRead; ++i) {
                uint8_t sym = static_cast<uint8_t>(readBuffer[i]);
                bitWriter.writeBits(codes[sym]);
            }
        }

        bitWriter.flush();
    }

    outFile.close();
    inFile.close();

    stats.compressedSize = FileUtils::getFileSize(stats.outputPath);

    if (stats.originalSize > 0 && stats.compressedSize > 0) {
        stats.compressionRatio = static_cast<double>(stats.originalSize) / stats.compressedSize;
        stats.spaceSavings = (1.0 - static_cast<double>(stats.compressedSize) / stats.originalSize) * 100.0;
    } else {
        stats.compressionRatio = 1.0;
        stats.spaceSavings = 0.0;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    stats.durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return stats;
}

CompressionStats HuffmanCompressor::decompress(const std::string& inputPath, const std::string& outputPath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    CompressionStats stats;
    stats.inputPath = inputPath;
    stats.outputPath = outputPath.empty() ? FileUtils::getDecompressedPath(inputPath) : outputPath;

    if (!FileUtils::exists(inputPath) || !FileUtils::isRegularFile(inputPath)) {
        throw std::runtime_error("Input file does not exist or is not a regular file: " + inputPath);
    }

    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open compressed file: " + inputPath);
    }

    // Read and verify header
    char magic[4];
    if (!inFile.read(magic, 4) ||
        magic[0] != MAGIC[0] || magic[1] != MAGIC[1] ||
        magic[2] != MAGIC[2] || magic[3] != MAGIC[3]) {
        throw std::runtime_error("Invalid file header: Not a recognized Huffman compressed file (.huf).");
    }

    if (!readBinary<uint64_t>(inFile, stats.originalSize)) {
        throw std::runtime_error("Corrupted header: unable to read original size.");
    }

    if (!readBinary<uint16_t>(inFile, stats.distinctSymbols)) {
        throw std::runtime_error("Corrupted header: unable to read distinct symbol count.");
    }

    stats.compressedSize = FileUtils::getFileSize(inputPath);

    std::ofstream outFile(stats.outputPath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open output file for decompression: " + stats.outputPath);
    }

    // Handle empty file
    if (stats.originalSize > 0) {
        BitReader bitReader(inFile);
        HuffmanTree tree;
        tree.deserialize(bitReader, stats.distinctSymbols);

        std::vector<char> writeBuffer;
        writeBuffer.reserve(65536);

        for (uint64_t i = 0; i < stats.originalSize; ++i) {
            uint8_t sym = 0;
            if (!tree.decodeSymbol(bitReader, sym)) {
                throw std::runtime_error("Premature end of file or corrupted bitstream during decompression.");
            }
            writeBuffer.push_back(static_cast<char>(sym));

            if (writeBuffer.size() >= 65536) {
                outFile.write(writeBuffer.data(), writeBuffer.size());
                writeBuffer.clear();
            }
        }

        if (!writeBuffer.empty()) {
            outFile.write(writeBuffer.data(), writeBuffer.size());
        }
    }

    outFile.close();
    inFile.close();

    if (stats.originalSize > 0 && stats.compressedSize > 0) {
        stats.compressionRatio = static_cast<double>(stats.originalSize) / stats.compressedSize;
        stats.spaceSavings = (1.0 - static_cast<double>(stats.compressedSize) / stats.originalSize) * 100.0;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    stats.durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    return stats;
}

