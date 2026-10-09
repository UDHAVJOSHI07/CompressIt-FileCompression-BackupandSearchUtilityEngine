#include "hash.h"

#include <iomanip>
#include <sstream>

namespace dupdetect {

void Fnv1a64::update(const unsigned char* data, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i) {
        state_ ^= data[i];
        state_ *= kPrime;
    }
}

void Fnv1a64::update(const std::string& text) {
    update(reinterpret_cast<const unsigned char*>(text.data()), text.size());
}

std::uint64_t fnv1a64(const std::string& text) {
    Fnv1a64 hasher;
    hasher.update(text);
    return hasher.digest();
}

std::uint64_t fnv1a64(const std::vector<unsigned char>& bytes) {
    Fnv1a64 hasher;
    hasher.update(bytes.data(), bytes.size());
    return hasher.digest();
}

std::string toHex(std::uint64_t value) {
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

}
