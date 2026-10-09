#include "file_reader.h"
#include "hash.h"
#include "hasher.h"
#include "test_framework.h"
#include "test_helpers.h"

#include <string>
#include <vector>

using namespace dupdetect;
using testutil::freshTempDir;
using testutil::writeFile;

TEST_CASE(empty_file_reads_as_zero_bytes) {
    const fs::path dir = freshTempDir("empty_read");
    writeFile(dir / "empty.txt", "");
    const ReadResult result = readWholeFile(dir / "empty.txt");
    CHECK(result.ok);
    CHECK(result.data.empty());
}

TEST_CASE(binary_content_is_read_exactly) {
    const fs::path dir = freshTempDir("binary_read");
    const std::string content("A\0B\xFF\r\nC", 7);
    writeFile(dir / "data.bin", content);
    const ReadResult result = readWholeFile(dir / "data.bin");
    CHECK(result.ok);
    CHECK(std::string(result.data.begin(), result.data.end()) == content);
}

TEST_CASE(small_chunks_reassemble_the_whole_file) {
    const fs::path dir = freshTempDir("chunked_read");
    const std::string content = "0123456789abcdefghij";
    writeFile(dir / "data.txt", content);

    std::string collected;
    int chunks = 0;
    std::string error;
    const bool ok = readFileInChunks(
        dir / "data.txt",
        [&](const unsigned char* data, std::size_t size) {
            collected.append(reinterpret_cast<const char*>(data), size);
            ++chunks;
        },
        error, 7);
    CHECK(ok);
    CHECK_EQ(collected, content);
    CHECK_EQ(chunks, 3);
}

TEST_CASE(missing_file_reports_error) {
    const fs::path dir = freshTempDir("missing_read");
    const ReadResult result = readWholeFile(dir / "does_not_exist.txt");
    CHECK(!result.ok);
    CHECK(result.error.find("not found") != std::string::npos);
}

TEST_CASE(directory_is_rejected_as_a_file) {
    const fs::path dir = freshTempDir("dir_read");
    const ReadResult result = readWholeFile(dir);
    CHECK(!result.ok);
    CHECK(result.error.find("directory") != std::string::npos);
}

TEST_CASE(zero_chunk_size_is_rejected) {
    const fs::path dir = freshTempDir("zero_chunk");
    writeFile(dir / "a.txt", "abc");
    std::string error;
    const bool ok = readFileInChunks(
        dir / "a.txt", [](const unsigned char*, std::size_t) {}, error, 0);
    CHECK(!ok);
    CHECK(!error.empty());
}

TEST_CASE(hash_of_empty_file_is_offset_basis) {
    const fs::path dir = freshTempDir("hash_empty");
    writeFile(dir / "empty.txt", "");
    const FileHashResult result = hashFile(dir / "empty.txt");
    CHECK(result.ok);
    CHECK_EQ(result.hash, Fnv1a64::kOffsetBasis);
    CHECK_EQ(result.bytesRead, 0u);
}

TEST_CASE(file_hash_matches_string_hash) {
    const fs::path dir = freshTempDir("hash_matches");
    writeFile(dir / "foobar.txt", "foobar");
    const FileHashResult result = hashFile(dir / "foobar.txt");
    CHECK(result.ok);
    CHECK_EQ(result.hash, 0x85944171f73967e8ULL);
    CHECK_EQ(result.bytesRead, 6u);
}

TEST_CASE(identical_files_have_identical_hashes) {
    const fs::path dir = freshTempDir("hash_identical");
    writeFile(dir / "one.txt", "same content");
    writeFile(dir / "two.txt", "same content");
    CHECK_EQ(hashFile(dir / "one.txt").hash, hashFile(dir / "two.txt").hash);
}

TEST_CASE(different_files_have_different_hashes) {
    const fs::path dir = freshTempDir("hash_different");
    writeFile(dir / "one.txt", "content A");
    writeFile(dir / "two.txt", "content B");
    CHECK(hashFile(dir / "one.txt").hash != hashFile(dir / "two.txt").hash);
}

TEST_CASE(hashing_missing_file_fails_cleanly) {
    const fs::path dir = freshTempDir("hash_missing");
    const FileHashResult result = hashFile(dir / "nope.bin");
    CHECK(!result.ok);
    CHECK(!result.error.empty());
}
