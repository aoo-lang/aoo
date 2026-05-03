#pragma once
#include "Expressions.hpp"

namespace AOO::Parser {
    [[nodiscard]] inline u32 parseItem(Parser& p, u16 flags, bool memberScope) noexcept;
    [[nodiscard]] inline u32 parseDeclarationAfterType(Parser& p, u16 flags, u32 typeNode, u32 nameIdx, bool memberScope) noexcept;
    [[nodiscard]] inline u32 parseStatement(Parser& p) noexcept;
    [[nodiscard]] inline u32 parseBlock(Parser& p) noexcept;

    [[nodiscard]] inline u32 makeNode(Parser& p, NodeKind kind, u32 tokIdx, const vector<u32>& children, u16 flags = 0, u64 payload = 0) noexcept {
        const u32 firstChild = appendChildren(p.ast, children);
        return addNode(p.ast, ASTNode{
            .kind = kind, .flags = flags, .tokenIndex = tokIdx,
            .firstChild = firstChild, .childCount = static_cast<u32>(children.size()), .payload = payload});
    }

    [[nodiscard]] inline u32 makeLeaf(Parser& p, NodeKind kind, u32 tokIdx, u16 flags = 0, u64 payload = 0) noexcept {
        return addNode(p.ast, ASTNode{
            .kind = kind, .flags = flags, .tokenIndex = tokIdx,
            .firstChild = 0, .childCount = 0, .payload = payload});
    }

    [[nodiscard]] inline u32 parseExpressionStatement(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        if (match(p, TokenType::CH_SEMICOLON)) return makeLeaf(p, NodeKind::Block, tokIdx, FLAG_SYNTHETIC);
        const u32 expr = parseExpression(p);
        if (!match(p, TokenType::CH_SEMICOLON)) {
            if (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) {
                recordError(p, ErrorKind::ExpectedSemicolon);
                resyncToStatementBoundary(p);
            }
        }
        return makeNode(p, NodeKind::ExprStmt, tokIdx, vector<u32>{expr});
    }

    [[nodiscard]] inline u32 parseReturnStatement(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        vector<u32> children;
        if (!match(p, TokenType::CH_SEMICOLON)) {
            if (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) children.push_back(parseExpression(p));
            if (!match(p, TokenType::CH_SEMICOLON) && !check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) {
                recordError(p, ErrorKind::ExpectedSemicolon);
                resyncToStatementBoundary(p);
            }
        }
        return makeNode(p, NodeKind::ReturnStmt, tokIdx, children);
    }

    [[nodiscard]] inline u32 parseJumpStatement(Parser& p, NodeKind kind) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        u64 payload = 0;
        if (peekType(p) == TokenType::LT_LABEL) {
            payload = currentTokenIndex(p);
            advance(p);
        }
        if (!match(p, TokenType::CH_SEMICOLON)) {
            recordError(p, ErrorKind::ExpectedSemicolon);
            resyncToStatementBoundary(p);
        }
        return makeLeaf(p, kind, tokIdx, 0, payload);
    }

    [[nodiscard]] inline u32 parseHeadExpression(Parser& p) noexcept {
        const bool oldFlag = p.stopAtConditionBrace;
        p.stopAtConditionBrace = true;
        const u32 expr = parseExpression(p);
        p.stopAtConditionBrace = oldFlag;
        return expr;
    }

    [[nodiscard]] inline u32 parseBodyStatement(Parser& p) noexcept {
        if (check(p, TokenType::CH_LEFT_BRACE)) return parseBlock(p);
        return parseStatement(p);
    }

    [[nodiscard]] inline u32 parseIfStatement(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        u32 cond = 0;
        if (match(p, TokenType::CH_LEFT_PAREN)) {
            cond = parseExpression(p);
            expect(p, TokenType::CH_RIGHT_PAREN, ErrorKind::ExpectedRightParen);
        } else cond = parseHeadExpression(p);

        vector<u32> children;
        children.push_back(cond);
        children.push_back(parseBodyStatement(p));
        if (match(p, TokenType::KW_ELSE)) children.push_back(parseBodyStatement(p));
        return makeNode(p, NodeKind::IfStmt, tokIdx, children);
    }

    [[nodiscard]] inline u32 parseForStatement(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        vector<u32> children;
        if (!check(p, TokenType::CH_LEFT_BRACE)) {
            const u64 saved = p.cursor;
            if (tryParseType(p) && peekType(p) == TokenType::LT_IDENTIFIER && peekTypeAhead(p, 1) == TokenType::KW_IN) {
                p.cursor = saved;
                const u32 typeNode = parseType(p);
                const u32 nameIdx = currentTokenIndex(p);
                advance(p);
                advance(p);
                children.push_back(makeNode(p, NodeKind::VariableDecl, nameIdx, vector<u32>{typeNode}));
                children.push_back(parseHeadExpression(p));
            } else if (peekType(p) == TokenType::LT_IDENTIFIER && peekTypeAhead(p, 1) == TokenType::KW_IN) {
                const u32 name = parseExpression(p);
                advance(p);
                children.push_back(name);
                children.push_back(parseHeadExpression(p));
            } else {
                p.cursor = saved;
                children.push_back(parseHeadExpression(p));
            }
        }
        children.push_back(parseBodyStatement(p));
        return makeNode(p, NodeKind::ForStmt, tokIdx, children);
    }

    [[nodiscard]] inline u32 parseMatchStatement(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        vector<u32> children;
        children.push_back(parseHeadExpression(p));
        children.push_back(parseBodyStatement(p));
        return makeNode(p, NodeKind::MatchStmt, tokIdx, children);
    }

    [[nodiscard]] inline u32 parseBlock(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        expect(p, TokenType::CH_LEFT_BRACE, ErrorKind::UnexpectedToken, "expected '{' to start block");
        vector<u32> children;
        while (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) children.push_back(parseStatement(p));
        if (!match(p, TokenType::CH_RIGHT_BRACE)) recordError(p, ErrorKind::ExpectedRightBrace);
        return makeNode(p, NodeKind::Block, tokIdx, children);
    }

    [[nodiscard]] inline u32 parseStatement(Parser& p) noexcept {
        skipTrivia(p);
        switch (peekType(p)) {
            case TokenType::CH_LEFT_BRACE: return parseBlock(p);
            case TokenType::KW_IF: return parseIfStatement(p);
            case TokenType::KW_FOR: return parseForStatement(p);
            case TokenType::KW_MATCH: return parseMatchStatement(p);
            case TokenType::KW_RETURN: return parseReturnStatement(p);
            case TokenType::KW_BREAK: return parseJumpStatement(p, NodeKind::BreakStmt);
            case TokenType::KW_CONTINUE: return parseJumpStatement(p, NodeKind::ContinueStmt);
            case TokenType::MISC_EOF: return makeErrorNode(p);
            default: break;
        }

        const u64 saved = p.cursor;
        if (tryParseType(p) && peekType(p) == TokenType::LT_IDENTIFIER) {
            p.cursor = saved;
            const u32 typeNode = parseType(p);
            const u32 nameIdx = currentTokenIndex(p);
            advance(p);
            return parseDeclarationAfterType(p, 0, typeNode, nameIdx, false);
        }
        p.cursor = saved;
        return parseExpressionStatement(p);
    }
}