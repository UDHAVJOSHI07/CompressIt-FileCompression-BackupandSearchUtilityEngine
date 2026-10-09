#include "file_compare.h"

#include "file_reader.h"

#include <cstring>
#include <fstream>
#include <system_error>
#include <vector>

namespace dupdetect {

CompareResult compareFileContents(const fs::path& first, const fs::path& second) {
    CompareResult result;

    if (!checkRegularFile(first, result.error) || !checkRegularFile(second, result.error)) {
        return result;
    }

    std::error_code ec1;
    std::error_code ec2;
    const std::uintmax_t size1 = fs::file_size(first, ec1);
    const std::uintmax_t size2 = fs::file_size(second, ec2);
    if (ec1 || ec2) {
        const fs::path& bad = ec1 ? first : second;
        result.error = "cannot get size of " + bad.string() + ": " +
                       (ec1 ? ec1 : ec2).message();
        return result;
    }
    if (size1 != size2) {
        result.outcome = CompareOutcome::Different;
        return result;
    }

    std::ifstream in1(first, std::ios::binary);
    std::ifstream in2(second, std::ios::binary);
    if (!in1.is_open() || !in2.is_open()) {
        result.error = "cannot open file: " + (in1.is_open() ? second : first).string();
        return result;
    }

    std::vector<char> buffer1(kDefaultChunkSize);
    std::vector<char> buffer2(kDefaultChunkSize);
    const auto chunk = static_cast<std::streamsize>(kDefaultChunkSize);

    while (true) {
        in1.read(buffer1.data(), chunk);
        in2.read(buffer2.data(), chunk);
        const std::streamsize got1 = in1.gcount();
        const std::streamsize got2 = in2.gcount();

        if (in1.bad() || in2.bad()) {
            result.error = "error while reading " + (in1.bad() ? first : second).string();
            return result;
        }
        if (got1 != got2 ||
            std::memcmp(buffer1.data(), buffer2.data(), static_cast<std::size_t>(got1)) != 0) {
            result.outcome = CompareOutcome::Different;
            return result;
        }
        if (got1 == 0) {
            break;
        }
    }

    result.outcome = CompareOutcome::Identical;
    return result;
}

}
