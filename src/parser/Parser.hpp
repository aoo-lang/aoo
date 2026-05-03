#pragma once
#include <vector>

#include "../lexer/Lexer.hpp"
#include "ASTNodes.hpp"
#include "Declarations.hpp"
#include "Errors.hpp"
#include "State.hpp"
#include "Statements.hpp"

namespace AOO::Parser {
    using std::vector;

    inline vector<ParseError> errors;
    inline AST ast;

    inline void parse() noexcept {
        errors.clear();
        ast = AST{};
        ast.nodes.reserve(Lexer::tokens.size() / 2 + 8);
        ast.childIndices.reserve(Lexer::tokens.size() / 2 + 8);

        Parser p{.tokens = Lexer::tokens, .ast = ast, .errors = errors};
        vector<u32> children;
        while (!atEnd(p)) children.push_back(parseItem(p, 0, false));
        ast.root = makeNode(p, NodeKind::Module, 0, children);
    }
}