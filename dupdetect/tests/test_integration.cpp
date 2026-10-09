#include "cli.h"
#include "duplicate_finder.h"
#include "report.h"
#include "scanner.h"
#include "test_framework.h"
#include "test_helpers.h"

#include <sstream>
#include <string>

using namespace dupdetect;
using testutil::freshTempDir;
using testutil::writeFile;

namespace {

bool hasText(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

fs::path buildSampleTree(const std::string& name) {
    const fs::path dir = freshTempDir(name);
    const std::string photo("\x89PNG\0\x01\x02 fake image bytes", 23);
    writeFile(dir / "photo.jpg", photo);
    writeFile(dir / "backup" / "photo_copy.jpg", photo);
    writeFile(dir / "backup" / "old" / "photo_old.jpg", photo);
    writeFile(dir / "notes.txt", "meeting notes");
    writeFile(dir / "archive" / "notes.txt", "meeting notes");
    writeFile(dir / "same_size.txt", "meeting nOtes");
    writeFile(dir / "unique.txt", "nothing else like this");
    writeFile(dir / "empty1.txt", "");
    writeFile(dir / "archive" / "empty2.txt", "");
    return dir;
}

}

TEST_CASE(full_pipeline_finds_expected_groups) {
    const fs::path dir = buildSampleTree("int_pipeline");
    const ScanResult scan = scanDirectory(dir);
    CHECK(scan.ok);
    CHECK_EQ(scan.files.size(), 9u);

    const DuplicateReport report = findDuplicates(scan.files);
    CHECK_EQ(report.groups.size(), 2u);
    CHECK_EQ(report.emptyFilesSkipped, 2u);
    CHECK(report.errors.empty());
    if (report.groups.size() == 2) {
        CHECK_EQ(report.groups[0].files.size(), 3u);
        CHECK_EQ(report.groups[0].fileSize, 23u);
        CHECK_EQ(report.groups[1].files.size(), 2u);
        CHECK(report.groups[1].files[0] == (dir / "archive" / "notes.txt"));
        CHECK(report.groups[1].files[1] == (dir / "notes.txt"));
    }
}

TEST_CASE(non_recursive_pipeline_ignores_subfolders) {
    const fs::path dir = buildSampleTree("int_flat");
    ScanOptions options;
    options.recursive = false;
    const DuplicateReport report = findDuplicates(scanDirectory(dir, options).files);
    CHECK(report.groups.empty());
}

TEST_CASE(run_app_prints_groups_and_summary) {
    const fs::path dir = buildSampleTree("int_run_app");
    CliOptions options;
    options.directory = dir;
    std::ostringstream out;
    std::ostringstream err;

    CHECK_EQ(runApp(options, out, err), kExitSuccess);
    const std::string text = out.str();
    CHECK(hasText(text, "Found 2 group(s) of duplicate files."));
    CHECK(hasText(text, "Group 1: 3 identical files"));
    CHECK(hasText(text, (dir / "backup" / "old" / "photo_old.jpg").string()));
    CHECK(hasText(text, "Redundant copies    : 3"));
    CHECK(!hasText(text, "unique.txt"));
    CHECK(!hasText(text, "same_size.txt"));
    CHECK(err.str().empty());
}

TEST_CASE(run_app_with_include_empty_reports_empty_group) {
    const fs::path dir = buildSampleTree("int_empty");
    CliOptions options;
    options.directory = dir;
    options.finder.includeEmptyFiles = true;
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(runApp(options, out, err), kExitSuccess);
    CHECK(hasText(out.str(), "Found 3 group(s)"));
}

TEST_CASE(run_app_on_folder_without_duplicates) {
    const fs::path dir = freshTempDir("int_no_dups");
    writeFile(dir / "a.txt", "alpha");
    writeFile(dir / "b.txt", "beta");
    CliOptions options;
    options.directory = dir;
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(runApp(options, out, err), kExitSuccess);
    CHECK(hasText(out.str(), "No duplicate files found."));
}

TEST_CASE(run_app_on_empty_folder) {
    const fs::path dir = freshTempDir("int_empty_folder");
    CliOptions options;
    options.directory = dir;
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(runApp(options, out, err), kExitSuccess);
    CHECK(hasText(out.str(), "Files checked       : 0"));
}

TEST_CASE(run_app_with_missing_directory_fails) {
    const fs::path dir = freshTempDir("int_missing");
    CliOptions options;
    options.directory = dir / "missing";
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(runApp(options, out, err), kExitRuntimeError);
    CHECK(hasText(err.str(), "directory not found"));
    CHECK(out.str().empty());
}

TEST_CASE(run_app_with_file_instead_of_directory_fails) {
    const fs::path dir = freshTempDir("int_file_root");
    writeFile(dir / "file.txt", "x");
    CliOptions options;
    options.directory = dir / "file.txt";
    std::ostringstream out;
    std::ostringstream err;
    CHECK_EQ(runApp(options, out, err), kExitRuntimeError);
    CHECK(hasText(err.str(), "not a directory"));
}

TEST_CASE(parse_directory_and_flags) {
    const ParseResult r = parseArguments({"--no-recursive", "data", "--include-empty"});
    CHECK(r.ok);
    CHECK(r.options.directory == fs::path("data"));
    CHECK(!r.options.scan.recursive);
    CHECK(r.options.finder.includeEmptyFiles);
}

TEST_CASE(parse_defaults) {
    const ParseResult r = parseArguments({"data"});
    CHECK(r.ok);
    CHECK(r.options.scan.recursive);
    CHECK(!r.options.finder.includeEmptyFiles);
    CHECK(!r.options.showHelp);
}

TEST_CASE(parse_rejects_bad_input) {
    CHECK(!parseArguments({}).ok);
    CHECK(!parseArguments({"--bogus", "data"}).ok);
    CHECK(!parseArguments({"one", "two"}).ok);
    CHECK(!parseArguments({""}).ok);
}

TEST_CASE(parse_help_needs_no_directory) {
    const ParseResult r = parseArguments({"--help"});
    CHECK(r.ok);
    CHECK(r.options.showHelp);
}

TEST_CASE(format_bytes_uses_readable_units) {
    CHECK_EQ(formatBytes(0), std::string("0 B"));
    CHECK_EQ(formatBytes(1023), std::string("1023 B"));
    CHECK_EQ(formatBytes(1024), std::string("1.0 KiB"));
    CHECK_EQ(formatBytes(1536), std::string("1.5 KiB"));
    CHECK_EQ(formatBytes(5ULL * 1024 * 1024), std::string("5.0 MiB"));
}
