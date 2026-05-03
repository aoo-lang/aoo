#pragma once
#include <fstream>
#include <iostream>

#include "../boot/Arguments.hpp"
#include "../lexer/Lexer.hpp"
#include "../parser/Parser.hpp"

namespace AOO::Debug {
    typedef uint32_t u32;
    using std::ofstream, std::ostream, std::cout, std::cerr, Lexer::tokens;

    inline void dumpAstNode(ostream& out, const Parser::AST& ast, u32 nodeIdx, int depth) noexcept {
        const Parser::ASTNode& node = ast.nodes[nodeIdx];
        for (int i = 0; i < depth; i++) out << "  ";
        out << nodeIdx << ": " << Parser::tostring_NodeKind(node.kind)
            << " token=" << node.tokenIndex
            << " flags=" << node.flags
            << " payload=" << node.payload
            << '\n';
        for (const u32 child : Parser::childrenOf(ast, node)) dumpAstNode(out, ast, child, depth + 1);
    }

    inline void dumpAst(ostream& out) noexcept {
        if (!Parser::errors.empty()) {
            out << "Errors:\n";
            for (const Parser::ParseError& error : Parser::errors) {
                out << "  " << Parser::tostring_ErrorKind(error.kind)
                    << " token=" << error.tokenIndex;
                if (!error.detail.empty()) out << " detail=\"" << error.detail << '"';
                out << '\n';
            }
            out << "AST:\n";
        }
        if (!Parser::ast.nodes.empty()) dumpAstNode(out, Parser::ast, Parser::ast.root, 0);
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