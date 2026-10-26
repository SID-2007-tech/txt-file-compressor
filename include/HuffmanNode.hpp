#ifndef HUFFMAN_NODE_HPP
#define HUFFMAN_NODE_HPP

#include <cstdint>
#include <algorithm>

struct HuffmanNode {
    uint8_t symbol;
    uint64_t frequency;
    uint8_t minSymbol; // Minimum symbol in subtree for deterministic tie-breaking
    HuffmanNode* left;
    HuffmanNode* right;

    // Leaf node constructor
    HuffmanNode(uint8_t sym, uint64_t freq)
        : symbol(sym), frequency(freq), minSymbol(sym), left(nullptr), right(nullptr) {}

    // Internal node constructor
    HuffmanNode(HuffmanNode* l, HuffmanNode* r)
        : symbol(0),
          frequency((l ? l->frequency : 0) + (r ? r->frequency : 0)),
          minSymbol(std::min(l ? l->minSymbol : 255, r ? r->minSymbol : 255)),
          left(l),
          right(r) {}

    ~HuffmanNode() {
        delete left;
        delete right;
    }

    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

struct HuffmanNodeComparator {
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const {
        if (a->frequency != b->frequency) {
            return a->frequency > b->frequency; // Min-heap (smallest frequency first)
        }
        return a->minSymbol > b->minSymbol; // Deterministic tie-breaker
    }
};

#endif // HUFFMAN_NODE_HPP

