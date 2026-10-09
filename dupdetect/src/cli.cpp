#include "cli.h"

#include "report.h"

#include <exception>
#include <new>

namespace dupdetect {

ParseResult parseArguments(const std::vector<std::string>& args) {
    ParseResult result;
    bool haveDirectory = false;

    for (const std::string& arg : args) {
        if (arg == "-h" || arg == "--help") {
            result.options.showHelp = true;
        } else if (arg == "--no-recursive") {
            result.options.scan.recursive = false;
        } else if (arg == "--include-empty") {
            result.options.finder.includeEmptyFiles = true;
        } else if (!arg.empty() && arg[0] == '-') {
            result.error = "unknown option: " + arg;
            return result;
        } else if (arg.empty()) {
            result.error = "directory path must not be empty";
            return result;
        } else if (haveDirectory) {
            result.error = "only one directory can be scanned at a time (got a second: " + arg + ")";
            return result;
        } else {
            result.options.directory = arg;
            haveDirectory = true;
        }
    }

    if (!haveDirectory && !result.options.showHelp) {
        result.error = "no directory given";
        return result;
    }
    result.ok = true;
    return result;
}

void printUsage(std::ostream& out) {
    out << "Usage: dupdetect <directory> [options]\n"
        << "\n"
        << "Finds files with identical contents inside <directory>.\n"
        << "\n"
        << "Options:\n"
        << "  --no-recursive   only scan the top level of <directory>\n"
        << "  --include-empty  also report empty (0-byte) files as duplicates\n"
        << "  -h, --help       show this help\n"
        << "\n"
        << "Exit codes: 0 = finished, 1 = bad arguments, 2 = could not scan\n";
}

int runApp(const CliOptions& options, std::ostream& out, std::ostream& err) {
    try {
        const ScanResult scan = scanDirectory(options.directory, options.scan);
        if (!scan.ok) {
            printFileErrors(scan.errors, "error", err);
            return kExitRuntimeError;
        }

        const DuplicateReport report = findDuplicates(scan.files, options.finder);

        printFileErrors(scan.errors, "warning", err);
        printFileErrors(report.errors, "warning", err);

        out << "Scanning: " << options.directory.string()
            << (options.scan.recursive ? " (recursive)" : " (top level only)") << "\n";
        if (scan.skippedSymlinks > 0) {
            out << "Skipped " << scan.skippedSymlinks << " symbolic link(s).\n";
        }
        out << "\n";
        printReport(report, out);
        return kExitSuccess;
    } catch (const std::bad_alloc&) {
        err << "error: out of memory\n";
    } catch (const std::exception& e) {
        err << "error: unexpected failure: " << e.what() << "\n";
    }
    return kExitRuntimeError;
}

}
