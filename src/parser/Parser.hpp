#pragma once
#include "Declarations.hpp"

namespace AOO::Parser {
    [[nodiscard]] inline AST parse(const vector<Token>& tokens, ErrorList& errors) noexcept {
        errors.clear();
        AST ast;
        ast.nodes.reserve(tokens.size() / 2 + 8);
        ast.childIndices.reserve(tokens.size() / 2 + 8);

        Parser p{.tokens = tokens, .ast = ast, .errors = errors};
        vector<u32> children;
        while (!atEnd(p)) children.push_back(parseItem(p, 0, false));
        ast.root = makeNode(p, NodeKind::Module, 0, children);
        return ast;
    }
}