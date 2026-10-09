#include "report.h"

#include "hash.h"

#include <iomanip>
#include <sstream>

namespace dupdetect {

std::string formatBytes(std::uintmax_t bytes) {
    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    }
    const char* units[] = {"KiB", "MiB", "GiB", "TiB"};
    double value = static_cast<double>(bytes) / 1024.0;
    int unit = 0;
    while (value >= 1024.0 && unit < 3) {
        value /= 1024.0;
        ++unit;
    }
    std::ostringstream text;
    text << std::fixed << std::setprecision(1) << value << " " << units[unit];
    return text.str();
}

void printReport(const DuplicateReport& report, std::ostream& out) {
    std::size_t redundantFiles = 0;
    std::uintmax_t reclaimableBytes = 0;

    if (report.groups.empty()) {
        out << "No duplicate files found.\n";
    } else {
        out << "Found " << report.groups.size() << " group(s) of duplicate files.\n";
    }

    int groupNumber = 0;
    for (const DuplicateGroup& group : report.groups) {
        ++groupNumber;
        const std::size_t extraCopies = group.files.size() - 1;
        redundantFiles += extraCopies;
        reclaimableBytes += group.fileSize * extraCopies;

        out << "\nGroup " << groupNumber << ": " << group.files.size()
            << " identical files, " << formatBytes(group.fileSize) << " each"
            << " (FNV-1a 64: " << toHex(group.hash) << ")\n";
        int fileNumber = 0;
        for (const fs::path& file : group.files) {
            out << "  [" << ++fileNumber << "] " << file.string() << "\n";
        }
    }

    out << "\nSummary\n"
        << "  Files checked       : " << report.filesConsidered << "\n"
        << "  Empty files skipped : " << report.emptyFilesSkipped << "\n"
        << "  Files with errors   : " << report.errors.size() << "\n"
        << "  Duplicate groups    : " << report.groups.size() << "\n"
        << "  Redundant copies    : " << redundantFiles << "\n"
        << "  Reclaimable space   : " << formatBytes(reclaimableBytes) << "\n";
}

void printFileErrors(const std::vector<FileError>& errors, const std::string& label,
                     std::ostream& out) {
    for (const FileError& error : errors) {
        out << label << ": ";
        if (!error.path.empty()) {
            out << error.path.string() << ": ";
        }
        out << error.message << "\n";
    }
}

}
