#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace dupdetect {

class Fnv1a64 {
public:
    static constexpr std::uint64_t kOffsetBasis = 0xcbf29ce484222325ULL;
    static constexpr std::uint64_t kPrime = 0x100000001b3ULL;

    void update(const unsigned char* data, std::size_t size);
    void update(const std::string& text);

    std::uint64_t digest() const { return state_; }

    void reset() { state_ = kOffsetBasis; }

private:
    std::uint64_t state_ = kOffsetBasis;
};

std::uint64_t fnv1a64(const std::string& text);
std::uint64_t fnv1a64(const std::vector<unsigned char>& bytes);

std::string toHex(std::uint64_t value);

}
