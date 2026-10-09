#include "hasher.h"

#include "file_reader.h"
#include "hash.h"

namespace dupdetect {

FileHashResult hashFile(const fs::path& path) {
    FileHashResult result;
    Fnv1a64 hasher;

    result.ok = readFileInChunks(
        path,
        [&](const unsigned char* data, std::size_t size) {
            hasher.update(data, size);
            result.bytesRead += size;
        },
        result.error);

    if (result.ok) {
        result.hash = hasher.digest();
    }
    return result;
}

}
