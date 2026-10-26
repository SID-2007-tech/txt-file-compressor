#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include "HuffmanNode.hpp"
#include "BitStream.hpp"
#include <vector>
#include <string>
#include <array>
#include <cstdint>

class HuffmanTree {
public:
    HuffmanTree();
    ~HuffmanTree();

    // Prevent copying
    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree& operator=(const HuffmanTree&) = delete;

    /**
     * @brief Builds Huffman tree from byte frequencies.
     * @param frequencies Array of 256 frequencies.
     */
    void buildFromFrequencies(const std::array<uint64_t, 256>& frequencies);

    /**
     * @brief Generates prefix codes for each symbol present in the tree.
     * @return Array of 256 strings containing '0' and '1' codes.
     */
    std::array<std::string, 256> generateCodes() const;

    /**
     * @brief Serializes the tree structure using pre-order traversal into BitWriter.
     */
    void serialize(BitWriter& writer) const;

    /**
     * @brief Deserializes a tree structure from BitReader.
     * @param reader BitReader to read from.
     * @param distinctCount Number of distinct symbols.
     */
    void deserialize(BitReader& reader, uint16_t distinctCount);

    /**
     * @brief Decodes a single symbol by traversing the tree using reader.
     * @param reader BitReader to read bits from.
     * @param symbol Output byte symbol.
     * @return true if successfully decoded a leaf, false on EOF or invalid path.
     */
    bool decodeSymbol(BitReader& reader, uint8_t& symbol) const;

    /**
     * @brief Checks if the tree has a valid root.
     */
    bool isValid() const { return root != nullptr; }

    /**
     * @brief Returns root node pointer.
     */
    const HuffmanNode* getRoot() const { return root; }

private:
    HuffmanNode* root;

    void generateCodesHelper(const HuffmanNode* node, const std::string& currentCode, std::array<std::string, 256>& codes) const;
    void serializeHelper(const HuffmanNode* node, BitWriter& writer) const;
    HuffmanNode* deserializeHelper(BitReader& reader);
    void clear();
};

#endif // HUFFMAN_TREE_HPP

