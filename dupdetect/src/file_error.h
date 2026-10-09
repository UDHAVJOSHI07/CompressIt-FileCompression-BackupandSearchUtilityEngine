#pragma once

#include <filesystem>
#include <string>

namespace dupdetect {

struct FileError {
    std::filesystem::path path;
    std::string message;
};

}
