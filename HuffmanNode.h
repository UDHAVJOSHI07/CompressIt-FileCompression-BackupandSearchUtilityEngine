#ifndef HUFFMAN_NODE_H
#define HUFFMAN_NODE_H

struct HuffmanNode {
    unsigned char character;
    long long frequency;

    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(unsigned char ch, long long freq);

    HuffmanNode(HuffmanNode* leftNode, HuffmanNode* rightNode);

    bool isLeaf() const;
};

#endif