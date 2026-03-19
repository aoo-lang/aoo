#include <iostream>

#include "boot/parseArguments.hpp"
#include "debug/lexerTokens.hpp"
#include "currentFile.hpp"
#include "lexer/Lexer.hpp"

using std::cerr;

int main(int argc, char** argv) {
    const auto args = AO::parseArguments(argc, argv);
    if (!AO::readInFile(args.inputFile)) {
        cerr << "Error: Failed to read file \"" << args.inputFile << "\"\n";
        return 1000;
    }
    AO::Lexer::init();
    AO::Lexer::parse();
    AO::Debug::printLexerTokens(args);
    return 0;
}