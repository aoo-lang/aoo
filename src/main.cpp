#include <iostream>

#include "currentFile.hpp"
#include "lexer/Lexer.hpp"
#include "boot/parseArguments.hpp"

using std::cout, std::cerr;

int main(int argc, char** argv) {
    const auto args = AO::parseArguments(argc, argv);
    if (!AO::readInFile(args.inputFile)) {
        cerr << "Error: Failed to read file \"" << args.inputFile << "\"\n";
        return 1000;
    }
    AO::Lexer::init();
    AO::Lexer::parse();
    if (args.printTokens) for (const auto& token : AO::Lexer::tokens) cout << token << '\n';
    return 0;
}