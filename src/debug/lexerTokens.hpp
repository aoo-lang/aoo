#pragma once
#include <iostream>
#include <fstream>

#include "../boot/Arguments.hpp"
#include "../lexer/Lexer.hpp"

namespace AO::Debug {
    using std::ofstream, std::cout, std::cerr, AO::Lexer::tokens;

    inline void printLexerTokens(const AO::Arguments& args) noexcept {
        if (args.printTokens) {
            if (!args.printTokensFile.empty()) {
                ofstream out(args.printTokensFile);
                if (!out) {
                    cerr << "Error: Failed to open file \"" << args.printTokensFile << "\" for writing\n";
                    exit(1001);
                }
                for (const auto& token : tokens) out << token << '\n';
                out.close();
            }
            else for (const auto& token : tokens) cout << token << '\n';
        }
    }
}