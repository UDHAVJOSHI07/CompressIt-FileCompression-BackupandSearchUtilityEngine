#include "hash.h"
#include "test_framework.h"

#include <string>
#include <vector>

using namespace dupdetect;

TEST_CASE(empty_input_hashes_to_offset_basis) {
    CHECK_EQ(fnv1a64(std::string()), 0xcbf29ce484222325ULL);
    CHECK_EQ(fnv1a64(std::vector<unsigned char>()), 0xcbf29ce484222325ULL);
    CHECK_EQ(toHex(fnv1a64(std::string())), std::string("cbf29ce484222325"));
}

TEST_CASE(matches_published_reference_values) {
    CHECK_EQ(fnv1a64("a"), 0xaf63dc4c8601ec8cULL);
    CHECK_EQ(fnv1a64("foobar"), 0x85944171f73967e8ULL);
}

TEST_CASE(identical_content_gives_identical_hash) {
    const std::string text = "The quick brown fox jumps over the lazy dog";
    CHECK_EQ(fnv1a64(text), fnv1a64(std::string(text)));
}

TEST_CASE(different_content_gives_different_hash) {
    CHECK(fnv1a64("hello") != fnv1a64("hellp"));
    CHECK(fnv1a64("abc") != fnv1a64("acb"));
    CHECK(fnv1a64("abc") != fnv1a64("abcd"));
}

TEST_CASE(zero_bytes_are_part_of_the_content) {
    const std::vector<unsigned char> one = {0x00};
    const std::vector<unsigned char> two = {0x00, 0x00};
    CHECK(fnv1a64(one) != fnv1a64(std::vector<unsigned char>()));
    CHECK(fnv1a64(one) != fnv1a64(two));
}

TEST_CASE(incremental_updates_match_single_update) {
    Fnv1a64 hasher;
    hasher.update("hello ");
    hasher.update("");
    hasher.update("world");
    CHECK_EQ(hasher.digest(), fnv1a64("hello world"));
}

TEST_CASE(reset_restores_initial_state) {
    Fnv1a64 hasher;
    hasher.update("some data");
    hasher.reset();
    CHECK_EQ(hasher.digest(), Fnv1a64::kOffsetBasis);
}

TEST_CASE(hex_output_is_padded_to_16_digits) {
    CHECK_EQ(toHex(0), std::string("0000000000000000"));
    CHECK_EQ(toHex(255), std::string("00000000000000ff"));
}
