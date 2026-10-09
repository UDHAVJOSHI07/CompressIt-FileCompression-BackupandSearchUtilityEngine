#pragma once

#include "duplicate_finder.h"
#include "file_error.h"

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace dupdetect {

std::string formatBytes(std::uintmax_t bytes);

void printReport(const DuplicateReport& report, std::ostream& out);

void printFileErrors(const std::vector<FileError>& errors, const std::string& label,
                     std::ostream& out);

}
