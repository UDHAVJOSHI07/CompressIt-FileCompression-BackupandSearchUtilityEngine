#include "scanner.h"

#include <algorithm>
#include <system_error>

namespace dupdetect {

namespace {

void handleEntry(const fs::directory_entry& entry, ScanResult& result) {
    std::error_code ec;
    if (entry.is_symlink(ec)) {
        ++result.skippedSymlinks;
        return;
    }
    if (entry.is_regular_file(ec)) {
        result.files.push_back(entry.path());
        return;
    }
    if (ec) {
        result.errors.push_back({entry.path(), "cannot determine file type: " + ec.message()});
    }
}

bool isUnreadableDirectory(const fs::directory_entry& entry, ScanResult& result) {
    std::error_code ec;
    if (entry.is_symlink(ec) || !entry.is_directory(ec)) {
        return false;
    }
    fs::directory_iterator probe(entry.path(), ec);
    if (ec) {
        result.errors.push_back({entry.path(), "cannot open directory, skipped: " + ec.message()});
        return true;
    }
    return false;
}

bool validateRoot(const fs::path& root, ScanResult& result) {
    std::error_code ec;
    const fs::file_status status = fs::status(root, ec);
    if (status.type() == fs::file_type::not_found) {
        result.errors.push_back({root, "directory not found"});
        return false;
    }
    if (ec) {
        result.errors.push_back({root, "cannot access directory: " + ec.message()});
        return false;
    }
    if (!fs::is_directory(status)) {
        result.errors.push_back({root, "not a directory"});
        return false;
    }
    return true;
}

}

ScanResult scanDirectory(const fs::path& root, const ScanOptions& options) {
    ScanResult result;
    if (!validateRoot(root, result)) {
        return result;
    }

    std::error_code ec;
    if (options.recursive) {
        fs::recursive_directory_iterator it(root, ec);
        const fs::recursive_directory_iterator end;
        if (ec) {
            result.errors.push_back({root, "cannot open directory: " + ec.message()});
            return result;
        }
        while (it != end) {
            const fs::path current = it->path();
            handleEntry(*it, result);
            if (isUnreadableDirectory(*it, result)) {
                it.disable_recursion_pending();
            }
            it.increment(ec);
            if (ec) {
                result.errors.push_back({current, "scan stopped after this entry: " + ec.message()});
                break;
            }
        }
    } else {
        fs::directory_iterator it(root, ec);
        const fs::directory_iterator end;
        if (ec) {
            result.errors.push_back({root, "cannot open directory: " + ec.message()});
            return result;
        }
        while (it != end) {
            const fs::path current = it->path();
            handleEntry(*it, result);
            it.increment(ec);
            if (ec) {
                result.errors.push_back({current, "scan stopped after this entry: " + ec.message()});
                break;
            }
        }
    }

    std::sort(result.files.begin(), result.files.end());
    result.ok = true;
    return result;
}

}
