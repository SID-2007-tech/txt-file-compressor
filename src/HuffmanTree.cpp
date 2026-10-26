#include "HuffmanTree.hpp"
#include <queue>
#include <stdexcept>

HuffmanTree::HuffmanTree() : root(nullptr) {}

HuffmanTree::~HuffmanTree() {
    clear();
}

void HuffmanTree::clear() {
    delete root;
    root = nullptr;
}

void HuffmanTree::buildFromFrequencies(const std::array<uint64_t, 256>& frequencies) {
    clear();

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, HuffmanNodeComparator> pq;

    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            pq.push(new HuffmanNode(static_cast<uint8_t>(i), frequencies[i]));
        }
    }

    if (pq.empty()) {
        return; // Empty file
    }

    // Special case: only 1 distinct character
    if (pq.size() == 1) {
        root = pq.top();
        pq.pop();
        return;
    }

    // Standard Huffman tree construction (merging two smallest nodes)
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top();
        pq.pop();
        HuffmanNode* right = pq.top();
        pq.pop();

        HuffmanNode* parent = new HuffmanNode(left, right);
        pq.push(parent);
    }

    root = pq.top();
    pq.pop();
}

std::array<std::string, 256> HuffmanTree::generateCodes() const {
    std::array<std::string, 256> codes;
    if (!root) {
        return codes;
    }

    // If only 1 symbol exists, its prefix code is simply "0"
    if (root->isLeaf()) {
        codes[root->symbol] = "0";
        return codes;
    }

    generateCodesHelper(root, "", codes);
    return codes;
}

void HuffmanTree::generateCodesHelper(const HuffmanNode* node, const std::string& currentCode, std::array<std::string, 256>& codes) const {
    if (!node) return;

    if (node->isLeaf()) {
        codes[node->symbol] = currentCode;
        return;
    }

    generateCodesHelper(node->left, currentCode + "0", codes);
    generateCodesHelper(node->right, currentCode + "1", codes);
}

void HuffmanTree::serialize(BitWriter& writer) const {
    if (!root) return;

    if (root->isLeaf()) {
        // Distinct symbols == 1
        writer.writeByte(root->symbol);
        return;
    }

    serializeHelper(root, writer);
}

void HuffmanTree::serializeHelper(const HuffmanNode* node, BitWriter& writer) const {
    if (!node) return;

    if (node->isLeaf()) {
        writer.writeBit(1);
        writer.writeByte(node->symbol);
    } else {
        writer.writeBit(0);
        serializeHelper(node->left, writer);
        serializeHelper(node->right, writer);
    }
}

void HuffmanTree::deserialize(BitReader& reader, uint16_t distinctCount) {
    clear();

    if (distinctCount == 0) {
        return;
    }

    if (distinctCount == 1) {
        uint8_t sym = 0;
        if (!reader.readByte(sym)) {
            throw std::runtime_error("Corrupted header: failed to read single symbol.");
        }
        root = new HuffmanNode(sym, 0);
        return;
    }

    root = deserializeHelper(reader);
    if (!root) {
        throw std::runtime_error("Corrupted compressed data: failed to reconstruct Huffman tree.");
    }
}

HuffmanNode* HuffmanTree::deserializeHelper(BitReader& reader) {
    int bit = 0;
    if (!reader.readBit(bit)) {
        return nullptr;
    }

    if (bit == 1) {
        uint8_t sym = 0;
        if (!reader.readByte(sym)) {
            return nullptr;
        }
        return new HuffmanNode(sym, 0);
    } else {
        HuffmanNode* left = deserializeHelper(reader);
        HuffmanNode* right = deserializeHelper(reader);
        if (!left || !right) {
            delete left;
            delete right;
            return nullptr;
        }
        return new HuffmanNode(left, right);
    }
}

bool HuffmanTree::decodeSymbol(BitReader& reader, uint8_t& symbol) const {
    if (!root) return false;

    // Single distinct character case
    if (root->isLeaf()) {
        int bit = 0;
        if (!reader.readBit(bit)) {
            return false;
        }
        symbol = root->symbol;
        return true;
    }

    const HuffmanNode* current = root;
    while (current && !current->isLeaf()) {
        int bit = 0;
        if (!reader.readBit(bit)) {
            return false;
        }
        current = (bit == 0) ? current->left : current->right;
    }

    if (current && current->isLeaf()) {
        symbol = current->symbol;
        return true;
    }

    return false;
}

