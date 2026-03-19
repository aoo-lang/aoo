#pragma once
#include <string>

namespace AO {
    using std::string;

    struct Arguments {
        bool printTokens, printAST;
        string inputFile, printTokensFile;
    };
}