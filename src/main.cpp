#include <iostream>

#include "boot/parseArguments.hpp"
#include "currentFile.hpp"
#include "debug/dump.hpp"
#include "lexer/Lexer.hpp"

using std::cerr;

int main(int argc, char** argv) {
    const auto args = AOO::parseArguments(argc, argv);
    if (!AOO::parseFileContent(args)) {
        cerr << "Error: Failed to read input file or literal code.\n";
        exit(1001);
    }
    AOO::Lexer::init();
    AOO::Lexer::parse();
    AOO::Debug::dump(args);
    return 0;
}