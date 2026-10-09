#include "scanner.h"
#include "test_framework.h"
#include "test_helpers.h"

#include <algorithm>
#include <iostream>
#include <system_error>

using namespace dupdetect;
using testutil::freshTempDir;
using testutil::writeFile;

namespace {
bool contains(const std::vector<fs::path>& files, const fs::path& wanted) {
    return std::find(files.begin(), files.end(), wanted) != files.end();
}
}

TEST_CASE(recursive_scan_finds_nested_files) {
    const fs::path dir = freshTempDir("scan_recursive");
    writeFile(dir / "top.txt", "1");
    writeFile(dir / "sub" / "mid.txt", "2");
    writeFile(dir / "sub" / "deeper" / "low.txt", "3");
    const ScanResult result = scanDirectory(dir);
    CHECK(result.ok);
    CHECK_EQ(result.files.size(), 3u);
    CHECK(contains(result.files, dir / "sub" / "deeper" / "low.txt"));
    CHECK(result.errors.empty());
}

TEST_CASE(non_recursive_scan_stays_at_top_level) {
    const fs::path dir = freshTempDir("scan_flat");
    writeFile(dir / "top.txt", "1");
    writeFile(dir / "sub" / "mid.txt", "2");
    ScanOptions options;
    options.recursive = false;
    const ScanResult result = scanDirectory(dir, options);
    CHECK(result.ok);
    CHECK_EQ(result.files.size(), 1u);
    CHECK(contains(result.files, dir / "top.txt"));
}

TEST_CASE(directories_are_not_returned_as_files) {
    const fs::path dir = freshTempDir("scan_dirs_only");
    fs::create_directories(dir / "a" / "b");
    const ScanResult result = scanDirectory(dir);
    CHECK(result.ok);
    CHECK(result.files.empty());
}

TEST_CASE(results_are_sorted) {
    const fs::path dir = freshTempDir("scan_sorted");
    writeFile(dir / "c.txt", "c");
    writeFile(dir / "a.txt", "a");
    writeFile(dir / "b.txt", "b");
    const ScanResult result = scanDirectory(dir);
    CHECK(std::is_sorted(result.files.begin(), result.files.end()));
}

TEST_CASE(missing_root_is_fatal_error) {
    const fs::path dir = freshTempDir("scan_missing");
    const ScanResult result = scanDirectory(dir / "not_here");
    CHECK(!result.ok);
    CHECK_EQ(result.errors.size(), 1u);
}

TEST_CASE(file_as_root_is_fatal_error) {
    const fs::path dir = freshTempDir("scan_file_root");
    writeFile(dir / "plain.txt", "x");
    const ScanResult result = scanDirectory(dir / "plain.txt");
    CHECK(!result.ok);
    CHECK(!result.errors.empty() && result.errors[0].message == "not a directory");
}

TEST_CASE(symlinks_are_skipped) {
    const fs::path dir = freshTempDir("scan_symlink");
    writeFile(dir / "real.txt", "data");
    std::error_code ec;
    fs::create_symlink(dir / "real.txt", dir / "link.txt", ec);
    if (ec) {
        std::cout << "  (skipped: cannot create symlink here: " << ec.message() << ")\n";
        return;
    }
    const ScanResult result = scanDirectory(dir);
    CHECK_EQ(result.files.size(), 1u);
    CHECK_EQ(result.skippedSymlinks, 1u);
}
