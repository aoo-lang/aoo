#pragma once
#include <string>

namespace AOO {
    using std::string;

    struct Arguments {
        bool printLexerTokens;
        string inputFile, printLexerTokensFile;
    };
}