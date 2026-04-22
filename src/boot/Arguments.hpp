#pragma once
#include <string>
#include <vector>

namespace AOO {
    typedef uint8_t u8;
    using std::string, std::vector;

    struct Arguments {
        struct Debug {
            enum struct DumpKind : u8 {
                None, Lexer, AST, IR, ASM
            } dump_kind{DumpKind::None};
            string dump_to;
        } debug;
        string input_file_path, literal_code;
        vector<string> include_files, include_paths;
    };
}