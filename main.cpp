#include <iostream>
#include "compression/HuffmanCompressor.h"

using namespace std;

int main() {

    HuffmanCompressor compressor;
    int ch; //this is choice variable
    while(1){
        cout<<"=============================\n\t\tCOMPRESSIT\n\t Compression Module\n=============================\n";  
        cout<<"1. Compress File\n2. Decompress File\n3. Exit\n";
        cout<<"\nEnter choice: ";
        cin>>ch;
        switch(ch){
            case 1:{
                string inpFile;
                string outFile;
                cout << "Enter input file: ";
                cin >> inpFile;
                cout << "Enter output file: ";
                cin >> outFile;
                compressor.compress(
                    inpFile,
                    outFile
                );
                break;
            }
            case 2:{
                string inpFile;
                string outFile;
                cout << "Enter compressed file: ";
                cin >> inpFile;
                cout << "Enter output file: ";
                cin >> outFile;
                compressor.decompress(
                    inpFile,
                    outFile
                );
                break;
            }
            case 3:
                cout << "Exiting CompressIt...\n";
                exit(0);
            default:
                cout << "Invalid choice.\n";
        }
    }
    return 0;
}