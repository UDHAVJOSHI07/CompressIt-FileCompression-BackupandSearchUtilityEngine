#pragma once

#include "duplicate_finder.h"
#include "scanner.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace dupdetect {

constexpr int kExitSuccess = 0;
constexpr int kExitUsageError = 1;
constexpr int kExitRuntimeError = 2;

struct CliOptions {
    std::filesystem::path directory;
    ScanOptions scan;
    FinderOptions finder;
    bool showHelp = false;
};

struct ParseResult {
    bool ok = false;
    CliOptions options;
    std::string error;
};

ParseResult parseArguments(const std::vector<std::string>& args);

void printUsage(std::ostream& out);

int runApp(const CliOptions& options, std::ostream& out, std::ostream& err);

}
