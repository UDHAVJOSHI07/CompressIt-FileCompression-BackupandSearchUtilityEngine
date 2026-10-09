#include "duplicate_finder.h"
#include "file_compare.h"
#include "test_framework.h"
#include "test_helpers.h"

#include <string>
#include <vector>

using namespace dupdetect;
using testutil::freshTempDir;
using testutil::writeFile;

TEST_CASE(compare_identical_files) {
    const fs::path dir = freshTempDir("cmp_identical");
    writeFile(dir / "a.bin", std::string("x\0y", 3));
    writeFile(dir / "b.bin", std::string("x\0y", 3));
    CHECK(compareFileContents(dir / "a.bin", dir / "b.bin").outcome == CompareOutcome::Identical);
}

TEST_CASE(compare_same_size_different_bytes) {
    const fs::path dir = freshTempDir("cmp_same_size");
    writeFile(dir / "a.txt", "abcd");
    writeFile(dir / "b.txt", "abce");
    CHECK(compareFileContents(dir / "a.txt", dir / "b.txt").outcome == CompareOutcome::Different);
}

TEST_CASE(compare_different_sizes) {
    const fs::path dir = freshTempDir("cmp_sizes");
    writeFile(dir / "a.txt", "abc");
    writeFile(dir / "b.txt", "abcd");
    CHECK(compareFileContents(dir / "a.txt", dir / "b.txt").outcome == CompareOutcome::Different);
}

TEST_CASE(compare_large_files_spanning_many_chunks) {
    const fs::path dir = freshTempDir("cmp_large");
    std::string big(200000, 'z');
    writeFile(dir / "a.bin", big);
    writeFile(dir / "b.bin", big);
    CHECK(compareFileContents(dir / "a.bin", dir / "b.bin").outcome == CompareOutcome::Identical);
    big[150000] = 'y';
    writeFile(dir / "c.bin", big);
    CHECK(compareFileContents(dir / "a.bin", dir / "c.bin").outcome == CompareOutcome::Different);
}

TEST_CASE(compare_with_missing_file_is_error) {
    const fs::path dir = freshTempDir("cmp_missing");
    writeFile(dir / "a.txt", "abc");
    const CompareResult result = compareFileContents(dir / "a.txt", dir / "nope.txt");
    CHECK(result.outcome == CompareOutcome::Error);
    CHECK(!result.error.empty());
}

TEST_CASE(two_identical_files_form_one_group) {
    const fs::path dir = freshTempDir("dup_pair");
    writeFile(dir / "a.txt", "hello");
    writeFile(dir / "b.txt", "hello");
    const DuplicateReport report = findDuplicates({dir / "a.txt", dir / "b.txt"});
    CHECK_EQ(report.groups.size(), 1u);
    if (report.groups.size() == 1) {
        CHECK_EQ(report.groups[0].files.size(), 2u);
        CHECK_EQ(report.groups[0].fileSize, 5u);
    }
    CHECK(report.errors.empty());
}

TEST_CASE(different_files_are_not_duplicates) {
    const fs::path dir = freshTempDir("dup_none");
    writeFile(dir / "a.txt", "hello");
    writeFile(dir / "b.txt", "world");
    writeFile(dir / "c.txt", "longer");
    const DuplicateReport report =
        findDuplicates({dir / "a.txt", dir / "b.txt", dir / "c.txt"});
    CHECK(report.groups.empty());
}

TEST_CASE(three_copies_form_a_single_group) {
    const fs::path dir = freshTempDir("dup_three");
    writeFile(dir / "a.txt", "copy");
    writeFile(dir / "b.txt", "copy");
    writeFile(dir / "c.txt", "copy");
    const DuplicateReport report =
        findDuplicates({dir / "c.txt", dir / "a.txt", dir / "b.txt"});
    CHECK_EQ(report.groups.size(), 1u);
    if (report.groups.size() == 1) {
        CHECK_EQ(report.groups[0].files.size(), 3u);
        CHECK(report.groups[0].files.front() == (dir / "a.txt"));
    }
}

TEST_CASE(separate_groups_are_kept_apart_and_ordered_by_size) {
    const fs::path dir = freshTempDir("dup_two_groups");
    writeFile(dir / "s1.txt", "ab");
    writeFile(dir / "s2.txt", "ab");
    writeFile(dir / "l1.txt", "abcdef");
    writeFile(dir / "l2.txt", "abcdef");
    const DuplicateReport report = findDuplicates(
        {dir / "s1.txt", dir / "l1.txt", dir / "s2.txt", dir / "l2.txt"});
    CHECK_EQ(report.groups.size(), 2u);
    if (report.groups.size() == 2) {
        CHECK_EQ(report.groups[0].fileSize, 6u);
        CHECK_EQ(report.groups[1].fileSize, 2u);
    }
}

TEST_CASE(empty_files_skipped_by_default) {
    const fs::path dir = freshTempDir("dup_empty_default");
    writeFile(dir / "e1.txt", "");
    writeFile(dir / "e2.txt", "");
    const DuplicateReport report = findDuplicates({dir / "e1.txt", dir / "e2.txt"});
    CHECK(report.groups.empty());
    CHECK_EQ(report.emptyFilesSkipped, 2u);
}

TEST_CASE(empty_files_grouped_when_enabled) {
    const fs::path dir = freshTempDir("dup_empty_enabled");
    writeFile(dir / "e1.txt", "");
    writeFile(dir / "e2.txt", "");
    FinderOptions options;
    options.includeEmptyFiles = true;
    const DuplicateReport report = findDuplicates({dir / "e1.txt", dir / "e2.txt"}, options);
    CHECK_EQ(report.groups.size(), 1u);
}

TEST_CASE(hash_collision_is_caught_by_byte_comparison) {
    const fs::path dir = freshTempDir("dup_collision");
    writeFile(dir / "a.txt", "AAAA");
    writeFile(dir / "b.txt", "BBBB");
    writeFile(dir / "c.txt", "AAAA");

    FinderOptions options;
    options.hashFunction = [](const fs::path&) {
        FileHashResult fake;
        fake.ok = true;
        fake.hash = 42;
        return fake;
    };
    const DuplicateReport report =
        findDuplicates({dir / "a.txt", dir / "b.txt", dir / "c.txt"}, options);

    CHECK_EQ(report.groups.size(), 1u);
    if (report.groups.size() == 1) {
        CHECK_EQ(report.groups[0].files.size(), 2u);
        CHECK(report.groups[0].files[0] == (dir / "a.txt"));
        CHECK(report.groups[0].files[1] == (dir / "c.txt"));
    }
}

TEST_CASE(same_path_listed_twice_is_not_a_duplicate) {
    const fs::path dir = freshTempDir("dup_same_path");
    writeFile(dir / "a.txt", "only one file");
    const DuplicateReport report =
        findDuplicates({dir / "a.txt", dir / "." / "a.txt"});
    CHECK(report.groups.empty());
    CHECK_EQ(report.filesConsidered, 1u);
}

TEST_CASE(missing_file_is_reported_and_others_still_checked) {
    const fs::path dir = freshTempDir("dup_missing");
    writeFile(dir / "a.txt", "data");
    writeFile(dir / "b.txt", "data");
    const DuplicateReport report =
        findDuplicates({dir / "a.txt", dir / "ghost.txt", dir / "b.txt"});
    CHECK_EQ(report.groups.size(), 1u);
    CHECK_EQ(report.errors.size(), 1u);
}

TEST_CASE(unique_sizes_are_never_hashed) {
    const fs::path dir = freshTempDir("dup_no_hash");
    writeFile(dir / "a.txt", "1");
    writeFile(dir / "b.txt", "22");
    writeFile(dir / "c.txt", "333");
    const DuplicateReport report =
        findDuplicates({dir / "a.txt", dir / "b.txt", dir / "c.txt"});
    CHECK_EQ(report.filesHashed, 0u);
}
