#pragma once
#include <iostream>
#include <fstream>

#include "../boot/Arguments.hpp"
#include "../lexer/Lexer.hpp"
#include "../parser/Parser.hpp"

namespace AOO::Debug {
    typedef uint32_t u32;
    using std::ofstream, std::ostream, std::cout, std::cerr, AOO::Lexer::tokens;

    inline void dumpAstNode(ostream& out, const AOO::Parser::AST& ast, u32 nodeIdx, int depth) noexcept {
        const AOO::Parser::ASTNode& node = ast.nodes[nodeIdx];
        for (int i = 0; i < depth; i++) out << "  ";
        out << nodeIdx << ": " << AOO::Parser::tostring_NodeKind(node.kind)
            << " token=" << node.tokenIndex
            << " flags=" << node.flags
            << " payload=" << node.payload
            << '\n';
        for (const AOO::Parser::u32 child : AOO::Parser::childrenOf(ast, node)) dumpAstNode(out, ast, child, depth + 1);
    }

    inline void dumpAst(ostream& out) noexcept {
        AOO::Parser::ErrorList errors;
        const AOO::Parser::AST ast = AOO::Parser::parse(tokens, errors);
        if (!errors.empty()) {
            out << "Errors:\n";
            for (const AOO::Parser::ParseError& error : errors) {
                out << "  " << AOO::Parser::tostring_ErrorKind(error.kind)
                    << " token=" << error.tokenIndex;
                if (!error.detail.empty()) out << " detail=\"" << error.detail << '"';
                out << '\n';
            }
            out << "AST:\n";
        }
        if (!ast.nodes.empty()) dumpAstNode(out, ast, ast.root, 0);
    }

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
                if (!args.debug.dump_to.empty()) {
                    ofstream out(args.debug.dump_to);
                    if (!out) {
                        cerr << "Error: Failed to open file \"" << args.debug.dump_to << "\" for writing\n";
                        exit(1001);
                    }
                    dumpAst(out);
                    out.close();
                }
                else dumpAst(cout);
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