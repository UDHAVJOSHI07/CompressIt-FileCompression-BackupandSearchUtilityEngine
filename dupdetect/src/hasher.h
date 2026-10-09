#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace dupdetect {

namespace fs = std::filesystem;

struct FileHashResult {
    bool ok = false;
    std::uint64_t hash = 0;
    std::uintmax_t bytesRead = 0;
    std::string error;
};

FileHashResult hashFile(const fs::path& path);

}
