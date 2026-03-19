#pragma once
#include <string>

namespace AOO {
    using std::string;

    struct Arguments {
        bool printLexerTokens, debugMode;
        string inputFile, printLexerTokensFile;
    };
}