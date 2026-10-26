#ifndef HUFFMAN_COMPRESSOR_HPP
#define HUFFMAN_COMPRESSOR_HPP

#include <string>
#include <cstdint>
#include <chrono>

struct CompressionStats {
    std::string inputPath;
    std::string outputPath;
    uint64_t originalSize = 0;
    uint64_t compressedSize = 0;
    uint16_t distinctSymbols = 0;
    double compressionRatio = 0.0; // originalSize / compressedSize
    double spaceSavings = 0.0;     // (1 - compressedSize / originalSize) * 100%
    double durationMs = 0.0;
};

class HuffmanCompressor {
public:
    HuffmanCompressor() = default;

    /**
     * @brief Compresses a file using Huffman coding.
     * @param inputPath Path to the input uncompressed file.
     * @param outputPath Optional explicit output path. If empty, placed in the same directory as input file.
     * @return CompressionStats struct with details of the operation.
     */
    CompressionStats compress(const std::string& inputPath, const std::string& outputPath = "");

    /**
     * @brief Decompresses a .huf file back to uncompressed form.
     * @param inputPath Path to the compressed file.
     * @param outputPath Optional explicit output path. If empty, placed in the same directory as input file.
     * @return CompressionStats struct with details of the operation.
     */
    CompressionStats decompress(const std::string& inputPath, const std::string& outputPath = "");

private:
    static constexpr uint8_t MAGIC[4] = {'H', 'U', 'F', 0x01};
};

#endif // HUFFMAN_COMPRESSOR_HPP

