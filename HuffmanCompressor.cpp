#include "HuffmanCompressor.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>
using namespace std;
// heap comparator for HuffmanNode pointer
struct CompareNodes {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->frequency > b->frequency;
    }
};
// Constructor
HuffmanCompressor::HuffmanCompressor() {
    root = nullptr;
}

// Destructor
HuffmanCompressor::~HuffmanCompressor() {
    deleteTree(root);
}

// delete huffman tree
void HuffmanCompressor::deleteTree(HuffmanNode* node) {
    if (node == nullptr)
        return;

    deleteTree(node->left);
    deleteTree(node->right);

    delete node;
}

// frequency table
void HuffmanCompressor::buildFrequencyTable(
    const string& filename,
    unordered_map<unsigned char, long long>& frequency
) {
    ifstream file(filename, ios::binary);

    unsigned char ch;

    while (file.read(reinterpret_cast<char*>(&ch), 1)) {
        frequency[ch]++;
    }

    file.close();
}

// Huffman tree
void HuffmanCompressor::buildHuffmanTree(
    const unordered_map<unsigned char, long long>& frequency
) {
    // Free any previously built tree before overwriting root
    deleteTree(root);
    root = nullptr;
    priority_queue<
        HuffmanNode*,
        vector<HuffmanNode*>,
        CompareNodes
    > minHeap;
    // unordered_map iteration order depends on internal bucket layout,
    // which is influenced by insertion order. That order differs between
    // the frequency map built while scanning the input file (compress)
    // and the frequency map rebuilt from the serialized table
    // (decompress). If nodes were pushed in raw map-iteration order,
    // ties in frequency could be broken differently on each side,
    // producing two different (but same-code-length) trees, and
    // therefore wrong characters on decode. Sorting by character first
    // makes the push order -- and therefore the resulting tree --
    // identical on both sides.
    vector<pair<unsigned char, long long>> items(
        frequency.begin(), frequency.end()
    );
    sort(
        items.begin(),
        items.end(),
        [](const pair<unsigned char, long long>& a,
           const pair<unsigned char, long long>& b) {
            return a.first < b.first;
        }
    );
    for (const auto& item : items) {
        HuffmanNode* node =
            new HuffmanNode(item.first, item.second);

        minHeap.push(node);
    }
    if (minHeap.empty()) {
        root = nullptr;
        return;
    }
    while (minHeap.size() > 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();
        HuffmanNode* right = minHeap.top();
        minHeap.pop();
        HuffmanNode* parent =
            new HuffmanNode(left, right);
        minHeap.push(parent);
    }
    root = minHeap.top();
}

//this part will genrate huffman code for character in tree
void HuffmanCompressor::generateCodes(
    HuffmanNode* node,
    const string& code
) {
    if (node == nullptr)
        return;
    if (node->isLeaf()) {
        if (code.empty())
            codes[node->character] = "0";
        else
            codes[node->character] = code;
        return;
    }
    generateCodes(node->left, code + "0");
    generateCodes(node->right, code + "1");
}
// Compress file
bool HuffmanCompressor::compress(
    const string& inputFile,
    const string& outputFile
) {
    unordered_map<unsigned char, long long> frequency;
    buildFrequencyTable(inputFile, frequency);
    if (frequency.empty()) {
        cout << "Input file is empty.\n";
        return false;
    }
    buildHuffmanTree(frequency);
    codes.clear();
    generateCodes(root, "");
    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);
    if (!input || !output) {
        cout << "Error opening files.\n";
        return false;
    }
    // Store frequency table
    // Store original file size
    long long originalSize = 0;
    for (const auto& item : frequency) {
        originalSize += item.second;
    }
    output.write(
        reinterpret_cast<char*>(&originalSize),
        sizeof(originalSize)
    );
    // Store frequency table
    unsigned short tableSize =
        static_cast<unsigned short>(frequency.size());

    output.write(
        reinterpret_cast<char*>(&tableSize),
        sizeof(tableSize)
    );
    for (const auto& item : frequency) {
        unsigned char character = item.first;
        long long freq = item.second;

        output.write(
            reinterpret_cast<char*>(&character),
            sizeof(character)
        );

        output.write(
            reinterpret_cast<char*>(&freq),
            sizeof(freq)
        );
    }
    // Convert characters into bits
    unsigned char buffer = 0;
    int bitCount = 0;
    unsigned char ch;
    while (input.read(reinterpret_cast<char*>(&ch), 1)) {
        string code = codes[ch];
        for (char bit : code) {
            buffer <<= 1;
            if (bit == '1')
                buffer |= 1;
            bitCount++;
            if (bitCount == 8) {
                output.write(
                    reinterpret_cast<char*>(&buffer),
                    1
                );
                buffer = 0;
                bitCount = 0;
            }
        }
    }
    // Write remaining bits
    if (bitCount > 0) {
        buffer <<= (8 - bitCount);
        output.write(
            reinterpret_cast<char*>(&buffer),
            1
        );
    }
    input.close();
    output.close();
    cout << "Compression successful!\n";
    return true;
}
// Decompress file
bool HuffmanCompressor::decompress(
    const string& inputFile,
    const string& outputFile
) {
    ifstream input(inputFile, ios::binary);
    if (!input) {
        cout << "Cannot open compressed file.\n";
        return false;
    }
    // Read frequency table
    // Read original file size
    long long originalSize;
    input.read(
        reinterpret_cast<char*>(&originalSize),
        sizeof(originalSize)
    );
    if (!input) {
        cout << "Compressed file is truncated or corrupted.\n";
        return false;
    }
    // Read frequency table
    unsigned short tableSize;
    input.read(
        reinterpret_cast<char*>(&tableSize),
        sizeof(tableSize)
    );
    if (!input) {
        cout << "Compressed file is truncated or corrupted.\n";
        return false;
    }
    unordered_map<unsigned char, long long> frequency;
    for (int i = 0; i < tableSize; i++) {
        unsigned char character;
        long long freq;
        input.read(
            reinterpret_cast<char*>(&character),
            sizeof(character)
        );
        input.read(
            reinterpret_cast<char*>(&freq),
            sizeof(freq)
        );
        if (!input) {
            cout << "Compressed file is truncated or corrupted.\n";
            return false;
        }
        frequency[character] = freq;
    }
    // Rebuild Huffman tree
    buildHuffmanTree(frequency);
    ofstream output(outputFile, ios::binary);
    if (!output) {
        cout << "Cannot create output file.\n";
        return false;
    }
    // Special case: only one distinct byte value in the original file.
    // The tree is a single leaf with no children, so the normal
    // left/right traversal below would dereference a null pointer.
    if (root->isLeaf()) {
        for (long long i = 0; i < originalSize; i++) {
            output.put(static_cast<char>(root->character));
        }
        input.close();
        output.close();
        cout << "Decompression successful!\n";
        return true;
    }
    HuffmanNode* current = root;
    unsigned char byte;
    long long decodedCharacters = 0;
    while (
        decodedCharacters < originalSize &&
        input.read(reinterpret_cast<char*>(&byte), 1)
    ) {
        for (int i = 7; i >= 0 && decodedCharacters < originalSize; i--) {
            int bit = (byte >> i) & 1;
            if (bit == 0)
                current = current->left;
            else
                current = current->right;
            if (current->isLeaf()) {
                output.put(current->character);
                decodedCharacters++;
                current = root;
            }
        }
    }
    input.close();
    output.close();
    cout << "Decompression successful!\n";
    return true;
}