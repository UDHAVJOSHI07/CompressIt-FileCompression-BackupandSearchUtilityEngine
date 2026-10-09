#include "duplicate_finder.h"

#include "file_compare.h"
#include "file_reader.h"

#include <algorithm>
#include <map>
#include <set>
#include <system_error>

namespace dupdetect {

namespace {

std::vector<std::vector<fs::path>> splitByContent(const std::vector<fs::path>& candidates,
                                                  std::vector<FileError>& errors) {
    std::vector<std::vector<fs::path>> groups;
    for (const fs::path& file : candidates) {
        bool handled = false;
        for (std::vector<fs::path>& group : groups) {
            const CompareResult cmp = compareFileContents(group.front(), file);
            if (cmp.outcome == CompareOutcome::Identical) {
                group.push_back(file);
                handled = true;
                break;
            }
            if (cmp.outcome == CompareOutcome::Error) {
                errors.push_back({file, cmp.error});
                handled = true;
                break;
            }
        }
        if (!handled) {
            groups.push_back({file});
        }
    }
    return groups;
}

}

DuplicateReport findDuplicates(const std::vector<fs::path>& inputFiles,
                               const FinderOptions& options) {
    DuplicateReport report;
    const std::function<FileHashResult(const fs::path&)> hashFunction =
        options.hashFunction ? options.hashFunction : hashFile;

    std::vector<fs::path> files;
    std::set<fs::path> seen;
    for (const fs::path& path : inputFiles) {
        const fs::path normal = path.lexically_normal();
        if (seen.insert(normal).second) {
            files.push_back(normal);
        }
    }
    report.filesConsidered = files.size();

    std::map<std::uintmax_t, std::vector<fs::path>> bySize;
    for (const fs::path& path : files) {
        std::string error;
        if (!checkRegularFile(path, error)) {
            report.errors.push_back({path, error});
            continue;
        }
        std::error_code ec;
        const std::uintmax_t size = fs::file_size(path, ec);
        if (ec) {
            report.errors.push_back({path, "cannot get file size: " + ec.message()});
            continue;
        }
        if (size == 0 && !options.includeEmptyFiles) {
            ++report.emptyFilesSkipped;
            continue;
        }
        bySize[size].push_back(path);
    }

    for (const auto& [size, sameSize] : bySize) {
        if (sameSize.size() < 2) {
            continue;
        }

        std::map<std::uint64_t, std::vector<fs::path>> byHash;
        for (const fs::path& path : sameSize) {
            const FileHashResult hashed = hashFunction(path);
            if (!hashed.ok) {
                report.errors.push_back({path, hashed.error});
                continue;
            }
            ++report.filesHashed;
            byHash[hashed.hash].push_back(path);
        }

        for (const auto& [hash, candidates] : byHash) {
            if (candidates.size() < 2) {
                continue;
            }
            for (std::vector<fs::path>& group : splitByContent(candidates, report.errors)) {
                if (group.size() >= 2) {
                    std::sort(group.begin(), group.end());
                    report.groups.push_back({size, hash, std::move(group)});
                }
            }
        }
    }

    std::sort(report.groups.begin(), report.groups.end(),
              [](const DuplicateGroup& a, const DuplicateGroup& b) {
                  if (a.fileSize != b.fileSize) {
                      return a.fileSize > b.fileSize;
                  }
                  return a.files.front() < b.files.front();
              });
    return report;
}

}
