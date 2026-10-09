#ifndef HUFFMAN_COMPRESSOR_H
#define HUFFMAN_COMPRESSOR_H

#include "HuffmanNode.h"
#include <string>
#include <unordered_map>

class HuffmanCompressor {
private:
    HuffmanNode* root;

    std::unordered_map<unsigned char, std::string> codes;

    void buildFrequencyTable(
        const std::string& filename,
        std::unordered_map<unsigned char, long long>& frequency
    );

    void buildHuffmanTree(
        const std::unordered_map<unsigned char, long long>& frequency
    );

    void generateCodes(
        HuffmanNode* node,
        const std::string& code
    );

    void deleteTree(HuffmanNode* node);

public:
    HuffmanCompressor();
    ~HuffmanCompressor();

    bool compress(
        const std::string& inputFile,
        const std::string& outputFile
    );

    bool decompress(
        const std::string& inputFile,
        const std::string& outputFile
    );
};

#endif