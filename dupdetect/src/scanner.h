#pragma once

#include "file_error.h"

#include <cstddef>
#include <filesystem>
#include <vector>

namespace dupdetect {

namespace fs = std::filesystem;

struct ScanOptions {
    bool recursive = true;
};

struct ScanResult {
    bool ok = false;
    std::vector<fs::path> files;
    std::vector<FileError> errors;
    std::size_t skippedSymlinks = 0;
};

ScanResult scanDirectory(const fs::path& root, const ScanOptions& options = {});

}
