#include "HuffmanNode.h"

HuffmanNode::HuffmanNode(unsigned char ch, long long freq) {
    character = ch;
    frequency = freq;
    left = nullptr;
    right = nullptr;
}

HuffmanNode::HuffmanNode(
    HuffmanNode* leftNode,
    HuffmanNode* rightNode
) {
    character = 0;
    left = leftNode;
    right = rightNode;

    frequency = leftNode->frequency + rightNode->frequency;
}

bool HuffmanNode::isLeaf() const {
    return left == nullptr && right == nullptr;
}