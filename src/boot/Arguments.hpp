#pragma once
#include <string>

namespace AOO {
    typedef uint8_t u8;
    using std::string;

    struct Arguments {
        struct Debug {
            enum struct DumpKind : u8 {
                None, Lexer, AST, IR, ASM
            } dump_kind{DumpKind::None};
            string dump_to;
        } debug;
        string input_file_path, literal_code;
    };
}