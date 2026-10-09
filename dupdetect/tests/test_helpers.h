#pragma once

#include <filesystem>
#include <fstream>
#include <string>

namespace testutil {

namespace fs = std::filesystem;

inline fs::path freshTempDir(const std::string& name) {
    const fs::path dir = fs::temp_directory_path() / "dupdetect_tests" / name;
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir;
}

inline void writeFile(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

}
