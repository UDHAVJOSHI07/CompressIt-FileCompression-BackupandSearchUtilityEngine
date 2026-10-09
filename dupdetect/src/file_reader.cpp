#include "file_reader.h"

#include <fstream>
#include <system_error>

namespace dupdetect {

bool checkRegularFile(const fs::path& path, std::string& error) {
    std::error_code ec;
    const fs::file_status status = fs::status(path, ec);

    if (status.type() == fs::file_type::not_found) {
        error = "file not found: " + path.string();
        return false;
    }
    if (ec) {
        error = "cannot access " + path.string() + ": " + ec.message();
        return false;
    }
    if (fs::is_directory(status)) {
        error = "path is a directory, not a file: " + path.string();
        return false;
    }
    if (!fs::is_regular_file(status)) {
        error = "not a regular file: " + path.string();
        return false;
    }
    return true;
}

bool readFileInChunks(const fs::path& path,
                      const ChunkHandler& onChunk,
                      std::string& error,
                      std::size_t chunkSize) {
    error.clear();

    if (chunkSize == 0) {
        error = "chunk size must be greater than zero";
        return false;
    }
    if (!checkRegularFile(path, error)) {
        return false;
    }

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        error = "cannot open file (permission denied or file in use?): " + path.string();
        return false;
    }

    std::vector<char> buffer(chunkSize);
    while (in.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) ||
           in.gcount() > 0) {
        onChunk(reinterpret_cast<const unsigned char*>(buffer.data()),
                static_cast<std::size_t>(in.gcount()));
    }

    if (in.bad()) {
        error = "error while reading file: " + path.string();
        return false;
    }
    return true;
}

ReadResult readWholeFile(const fs::path& path) {
    ReadResult result;
    result.ok = readFileInChunks(
        path,
        [&result](const unsigned char* data, std::size_t size) {
            result.data.insert(result.data.end(), data, data + size);
        },
        result.error);

    if (!result.ok) {
        result.data.clear();
    }
    return result;
}

}
