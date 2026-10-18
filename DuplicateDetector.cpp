#include "DuplicateDetector.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

DuplicateDetector::DuplicateDetector() {
    wastedBytes = 0;
    filesScanned = 0;
}

// Walk the directory tree and bucket every regular file by its size
void DuplicateDetector::scanDirectory(const string& directory) {
    error_code ec;
    fs::recursive_directory_iterator it(
        directory, fs::directory_options::skip_permission_denied, ec);
    fs::recursive_directory_iterator end;

    for (; it != end && !ec; it.increment(ec)) {
        error_code fileEc;
        if (!it->is_regular_file(fileEc) || fileEc)
            continue;

        uintmax_t size = it->file_size(fileEc);
        if (fileEc || size == 0)      // ignore empty files
            continue;

        sizeGroups[size].push_back(it->path().string());
        filesScanned++;
    }
}

// 64-bit FNV-1a hash, streamed so large files don't fill memory
bool DuplicateDetector::hashFile(const string& path, uint64_t& hash) const {
    ifstream file(path, ios::binary);
    if (!file)
        return false;

    hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;

    char buffer[65536];
    while (file) {
        file.read(buffer, sizeof(buffer));
        streamsize n = file.gcount();
        for (streamsize i = 0; i < n; i++) {
            hash ^= static_cast<unsigned char>(buffer[i]);
            hash *= prime;
        }
    }
    return true;
}

// Final safety check: compare actual bytes (hash collisions are possible)
bool DuplicateDetector::filesAreIdentical(const string& a,
                                          const string& b) const {
    ifstream f1(a, ios::binary);
    ifstream f2(b, ios::binary);
    if (!f1 || !f2)
        return false;

    char buf1[65536];
    char buf2[65536];
    while (f1 && f2) {
        f1.read(buf1, sizeof(buf1));
        f2.read(buf2, sizeof(buf2));
        streamsize n1 = f1.gcount();
        streamsize n2 = f2.gcount();
        if (n1 != n2)
            return false;
        for (streamsize i = 0; i < n1; i++)
            if (buf1[i] != buf2[i])
                return false;
    }
    return true;
}

// Take files of the same size, hash them, verify matches
void DuplicateDetector::processSizeGroup(const vector<string>& files,
                                         uintmax_t fileSize) {
    // hash value -> files with that hash
    unordered_map<uint64_t, vector<string>> hashTable;

    for (const string& path : files) {
        uint64_t h;
        if (hashFile(path, h))
            hashTable[h].push_back(path);
    }

    for (const auto& entry : hashTable) {
        const vector<string>& bucket = entry.second;
        if (bucket.size() < 2)
            continue;

        // Split bucket into clusters of truly identical files
        vector<vector<string>> clusters;
        for (const string& path : bucket) {
            bool placed = false;
            for (auto& cluster : clusters) {
                if (filesAreIdentical(cluster[0], path)) {
                    cluster.push_back(path);
                    placed = true;
                    break;
                }
            }
            if (!placed)
                clusters.push_back({path});
        }

        for (const auto& cluster : clusters) {
            if (cluster.size() > 1) {
                duplicateGroups.push_back(cluster);
                wastedBytes += fileSize * (cluster.size() - 1);
            }
        }
    }
}

bool DuplicateDetector::findDuplicates(const string& directory) {
    error_code ec;
    if (!fs::is_directory(directory, ec)) {
        cout << "Directory not found: " << directory << "\n";
        return false;
    }

    sizeGroups.clear();
    duplicateGroups.clear();
    wastedBytes = 0;
    filesScanned = 0;

    scanDirectory(directory);

    for (const auto& group : sizeGroups) {
        if (group.second.size() > 1)          // only same-size files can match
            processSizeGroup(group.second, group.first);
    }
    return true;
}

void DuplicateDetector::printReport() const {
    cout << "\nFiles scanned: " << filesScanned << "\n";

    if (duplicateGroups.empty()) {
        cout << "No duplicate files found.\n";
        return;
    }

    int groupNo = 1;
    for (const auto& group : duplicateGroups) {
        cout << "\nGroup " << groupNo++ << " ("
             << group.size() << " identical files):\n";
        for (const string& path : group)
            cout << "  " << path << "\n";
    }
    cout << "\nDuplicate groups: " << duplicateGroups.size() << "\n";
    cout << "Wasted space: " << wastedBytes << " bytes ("
         << wastedBytes / 1024.0 << " KB)\n";
}

int DuplicateDetector::deleteDuplicates() {
    int deleted = 0;
    for (const auto& group : duplicateGroups) {
        for (size_t i = 1; i < group.size(); i++) {   // keep index 0
            error_code ec;
            if (fs::remove(group[i], ec) && !ec) {
                cout << "Deleted: " << group[i] << "\n";
                deleted++;
            } else {
                cout << "Could not delete: " << group[i] << "\n";
            }
        }
    }
    duplicateGroups.clear();
    wastedBytes = 0;
    return deleted;
}

const vector<vector<string>>& DuplicateDetector::getDuplicateGroups() const {
    return duplicateGroups;
}

uintmax_t DuplicateDetector::getWastedSpace() const {
    return wastedBytes;
}
