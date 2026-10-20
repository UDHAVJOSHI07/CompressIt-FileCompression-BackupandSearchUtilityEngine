#ifndef DUPLICATE_DETECTOR_H
#define DUPLICATE_DETECTOR_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Finds duplicate files inside a directory

class DuplicateDetector {
private:
    // size in bytes -> all files with that size
    std::unordered_map<std::uintmax_t, std::vector<std::string>> sizeGroups;

    // every iner vector = one set of identical files
    std::vector<std::vector<std::string>> duplicateGroups;

    std::uintmax_t wastedBytes;
    std::uintmax_t filesScanned;

    void scanDirectory(const std::string& directory);
    bool hashFile(const std::string& path, std::uint64_t& hash) const;
    bool filesAreIdentical(const std::string& a, const std::string& b) const;
    void processSizeGroup(const std::vector<std::string>& files,
                          std::uintmax_t fileSize);

public:
    DuplicateDetector();

    // scan directory and build duplicate groups. Returns false if the
    // directory does not exist / is not a directory.
    bool findDuplicates(const std::string& directory);

    void printReport() const;

    // keep the first file of each group and deletes the rest.
    // return number of file deleted.
    int deleteDuplicates();

    const std::vector<std::vector<std::string>>& getDuplicateGroups() const;
    std::uintmax_t getWastedSpace() const;
};

#endif
