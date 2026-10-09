#pragma once

#include "file_error.h"
#include "hasher.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>

namespace dupdetect {

namespace fs = std::filesystem;

struct DuplicateGroup {
    std::uintmax_t fileSize = 0;
    std::uint64_t hash = 0;
    std::vector<fs::path> files;
};

struct FinderOptions {
    bool includeEmptyFiles = false;

    std::function<FileHashResult(const fs::path&)> hashFunction = hashFile;
};

struct DuplicateReport {
    std::vector<DuplicateGroup> groups;
    std::vector<FileError> errors;
    std::size_t filesConsidered = 0;
    std::size_t filesHashed = 0;
    std::size_t emptyFilesSkipped = 0;
};

DuplicateReport findDuplicates(const std::vector<fs::path>& files,
                               const FinderOptions& options = {});

}
