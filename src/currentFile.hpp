#pragma once
#include <string>
#include <vector>

#include "boot/Arguments.hpp"
#include "util/filesystem.hpp"

namespace AOO {
    typedef uint8_t u8;
    using std::string, std::vector, Util::OS::readFile;

    inline vector<u8> fileContent;

    [[nodiscard]] inline bool parseFileContent(const Arguments& args) noexcept {
        if (!args.literal_code.empty()) {
            fileContent.assign(args.literal_code.begin(), args.literal_code.end());
            return true;
        }
        else if (!args.input_file_path.empty()) return readFile(args.input_file_path, fileContent);
        else return false;
    }
}