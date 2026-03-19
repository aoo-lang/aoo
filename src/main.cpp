#include <iostream>

#include "boot/parseArguments.hpp"
#include "debug/lexerTokens.hpp"
#include "currentFile.hpp"
#include "lexer/Lexer.hpp"

using std::cerr;

int main(int argc, char** argv) {
    const auto args = AOO::parseArguments(argc, argv);
    if (!AOO::readInFile(args.inputFile)) {
        cerr << "Error: Failed to read file \"" << args.inputFile << "\"\n";
        return 1000;
    }
    AOO::Lexer::init();
    AOO::Lexer::parse();
    AOO::Debug::printLexerTokens(args);
    return 0;
}