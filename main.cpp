#include <iostream>
#include <limits>
#include <string>

#include "DuplicateDetector.h"
#include "HuffmanCompressor.h"

using namespace std;

// Read a full line (so paths with spaces work)
string readLine(const string& prompt) {
    string line;
    cout << prompt;
    getline(cin, line);

    // remove quotes if the user pasted a path like "C:\my folder\file.txt"
    if (line.size() >= 2 && line.front() == '"' && line.back() == '"')
        line = line.substr(1, line.size() - 2);
    return line;
}

void compressMenu() {
    string input = readLine("Enter file to compress: ");
    string output = readLine("Enter output file name : ");

    HuffmanCompressor compressor;

    // Calculate original file size
    long long originalSize = compressor.getFileSize(input);

    if (originalSize < 0) {
        cout << "Cannot access input file.\n";
        return;
    }

    cout << "Original file size: "
         << originalSize / 1024.0 << " KB ("
         << originalSize << " bytes)\n";

    // Compress the file (prints "Compression successful!" itself)
    if (compressor.compress(input, output)) {
        long long compressedSize = compressor.getFileSize(output);

        if (compressedSize >= 0) {
            cout << "Compressed file size: "
                 << compressedSize / 1024.0 << " KB ("
                 << compressedSize << " bytes)\n";
        }
    } else {
        cout << "Compression failed.\n";
    }
}

void decompressMenu() {
    string input = readLine("Enter compressed file: ");
    string output = readLine("Enter output file name: ");

    HuffmanCompressor compressor;

    // Calculate compressed file size
    long long compressedSize = compressor.getFileSize(input);

    if (compressedSize < 0) {
        cout << "Cannot access compressed file.\n";
        return;
    }

    cout << "Compressed file size: "
         << compressedSize / 1024.0 << " KB ("
         << compressedSize << " bytes)\n";

    // Decompress the file (prints "Decompression successful!" itself)
    if (compressor.decompress(input, output)) {
        long long originalSize = compressor.getFileSize(output);

        if (originalSize >= 0) {
            cout << "Decompressed file size: "
                 << originalSize / 1024.0 << " KB ("
                 << originalSize << " bytes)\n";
        }
    } else {
        cout << "Decompression failed.\n";
    }
}

void duplicateMenu() {
    string dir = readLine("Enter folder path to scan: ");

    DuplicateDetector detector;
    if (!detector.findDuplicates(dir))
        return;

    detector.printReport();

    if (detector.getDuplicateGroups().empty())
        return;

    string choice = readLine(
        "\nDelete duplicates? First file in each group is kept (y/n): ");
    if (choice == "y" || choice == "Y") {
        int count = detector.deleteDuplicates();
        cout << count << " duplicate file(s) deleted.\n";
    }
}

void printMenu() {
    cout << "\n=============================\n";
    cout << "         CompressIt\n";
    cout << "=============================\n";
    cout << "1. Compress a file\n";
    cout << "2. Decompress a file\n";
    cout << "3. Find duplicate files\n";
    cout << "0. Exit\n";
    cout << "-----------------------------\n";
}

int main() {
    while (true) {
        printMenu();

        int choice;
        cout << "Enter choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Please enter a number.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear newline

        switch (choice) {
            case 1: compressMenu();   break;
            case 2: decompressMenu(); break;
            case 3: duplicateMenu();  break;
            case 0:
                cout << "Goodbye!\n";
                return 0;
            default:
                cout << "Invalid choice.\n";
        }
    }
}
