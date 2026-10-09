#pragma once

#include <filesystem>
#include <string>

namespace dupdetect {

namespace fs = std::filesystem;

enum class CompareOutcome {
    Identical,
    Different,
    Error
};

struct CompareResult {
    CompareOutcome outcome = CompareOutcome::Error;
    std::string error;
};

CompareResult compareFileContents(const fs::path& first, const fs::path& second);

}
