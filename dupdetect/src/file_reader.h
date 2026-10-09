#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace dupdetect {

namespace fs = std::filesystem;

using ChunkHandler = std::function<void(const unsigned char* data, std::size_t size)>;

constexpr std::size_t kDefaultChunkSize = 64 * 1024;

bool checkRegularFile(const fs::path& path, std::string& error);

bool readFileInChunks(const fs::path& path,
                      const ChunkHandler& onChunk,
                      std::string& error,
                      std::size_t chunkSize = kDefaultChunkSize);

struct ReadResult {
    bool ok = false;
    std::vector<unsigned char> data;
    std::string error;
};

ReadResult readWholeFile(const fs::path& path);

}
