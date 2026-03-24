#pragma once
#include <iostream>
#include <fstream>

#include "../boot/Arguments.hpp"
#include "../lexer/Lexer.hpp"

namespace AOO::Debug {
    using std::ofstream, std::cout, std::cerr, AOO::Lexer::tokens;

    inline void dump(const AOO::Arguments& args) noexcept {
        switch (args.debug.dump_kind) {
            using enum Arguments::Debug::DumpKind;
            case None: break;
            case Lexer:
                if (!args.debug.dump_to.empty()) {
                    ofstream out(args.debug.dump_to);
                    if (!out) {
                        cerr << "Error: Failed to open file \"" << args.debug.dump_to << "\" for writing\n";
                        exit(1001);
                    }
                    for (const auto& token : tokens) out << token << '\n';
                    out.close();
                }
                else for (const auto& token : tokens) cout << token << '\n';
                break;
            case AST:
                cerr << "Dumping AST is not implemented yet.\n";
                break;
            case IR:
                cerr << "Dumping IR is not implemented yet.\n";
                break;
            case ASM:
                cerr << "Dumping ASM is not implemented yet.\n";
                break;
        }
    }
}