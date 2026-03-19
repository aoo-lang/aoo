#pragma once
#include <iostream>
#include <fstream>

#include "../boot/Arguments.hpp"
#include "../lexer/Lexer.hpp"

namespace AOO::Debug {
    using std::ofstream, std::cout, std::cerr, AOO::Lexer::tokens;

    inline void printLexerTokens(const AOO::Arguments& args) noexcept {
        if (args.printLexerTokens) {
            if (!args.printLexerTokensFile.empty()) {
                ofstream out(args.printLexerTokensFile);
                if (!out) {
                    cerr << "Error: Failed to open file \"" << args.printLexerTokensFile << "\" for writing\n";
                    exit(1001);
                }
                for (const auto& token : tokens) out << token << '\n';
                out.close();
            }
            else for (const auto& token : tokens) cout << token << '\n';
        }
    }
}