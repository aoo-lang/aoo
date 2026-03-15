#pragma once
#include <string>
#include <vector>

#include "util/filesystem.hpp"

namespace AO {
    typedef uint8_t u8;
    using std::string, std::vector, Util::OS::readFile;

    inline vector<u8> fileContent;

    [[nodiscard]] inline bool readInFile(const string& filePath) noexcept {
        return readFile(filePath, fileContent);
    }
}