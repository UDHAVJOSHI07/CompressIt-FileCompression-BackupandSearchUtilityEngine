#include "cli.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    const dupdetect::ParseResult parsed = dupdetect::parseArguments(args);

    if (!parsed.ok) {
        std::cerr << "error: " << parsed.error << "\n\n";
        dupdetect::printUsage(std::cerr);
        return dupdetect::kExitUsageError;
    }
    if (parsed.options.showHelp) {
        dupdetect::printUsage(std::cout);
        return dupdetect::kExitSuccess;
    }
    return dupdetect::runApp(parsed.options, std::cout, std::cerr);
}
