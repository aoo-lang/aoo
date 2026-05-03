#pragma once
#include "Statements.hpp"

namespace AOO::Parser {
    struct GenericParseResult {
        vector<u32> nodes;
        u32 count{0};
    };

    struct BodyParseResult {
        u32 node{0};
        u64 payloadFlags{0};
    };

    [[nodiscard]] inline bool isOperatorNameToken(TokenType t) noexcept {
        using enum TokenType;
        switch (t) {
            case LT_IDENTIFIER:
            case OP_PLUS:
            case OP_DOUBLE_PLUS:
            case OP_DASH:
            case OP_DOUBLE_DASH:
            case OP_STAR:
            case OP_SLASH:
            case OP_PERCENT:
            case OP_LESS:
            case OP_LESS_EQUAL:
            case OP_DOUBLE_LESS_EQUAL:
            case OP_GREATER:
            case OP_GREATER_EQUAL:
            case OP_DOUBLE_GREATER_EQUAL:
            case OP_BAR:
            case OP_DOUBLE_BAR:
            case OP_AMPERSAND:
            case OP_DOUBLE_AMPERSAND:
            case OP_CARET:
            case OP_TILDE:
            case OP_BANG:
            case OP_DOUBLE_BANG:
            case OP_EQUAL:
            case OP_DOUBLE_EQUAL:
            case OP_BANG_EQUAL:
            case OP_QUESTION_COLON:
            case OP_DOUBLE_QUESTION:
            case OP_COLON:
            case OP_DOUBLE_COLON:
            case OP_PERIOD:
            case OP_DOUBLE_PERIOD:
            case OP_TRIPLE_PERIOD:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] inline u32 parsePathSegment(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        if (!expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier)) return makeErrorNode(p);
        return makeLeaf(p, NodeKind::ImportSegment, tokIdx);
    }

    [[nodiscard]] inline bool isPathSeparator(TokenType t) noexcept {
        return t == TokenType::OP_COLON || t == TokenType::OP_DOUBLE_COLON || t == TokenType::OP_PERIOD;
    }

    [[nodiscard]] inline u32 parsePath(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        vector<u32> children;
        children.push_back(parsePathSegment(p));
        while (isPathSeparator(peekType(p))) {
            advance(p);
            children.push_back(parsePathSegment(p));
        }
        return makeNode(p, NodeKind::ImportPath, tokIdx, children);
    }

    [[nodiscard]] inline vector<u32> parseImportPathParts(Parser& p, bool& hasGroupedItems) noexcept {
        vector<u32> parts;
        hasGroupedItems = false;
        if (peekType(p) != TokenType::LT_IDENTIFIER) {
            recordError(p, ErrorKind::ExpectedIdentifier, "expected import path segment");
            if (!atEnd(p)) advance(p);
            return parts;
        }
        parts.push_back(currentTokenIndex(p));
        advance(p);
        while (isPathSeparator(peekType(p))) {
            if (peekTypeAhead(p, 1) == TokenType::CH_LEFT_BRACE) {
                advance(p);
                hasGroupedItems = true;
                break;
            }
            advance(p);
            if (peekType(p) != TokenType::LT_IDENTIFIER) {
                recordError(p, ErrorKind::ExpectedIdentifier, "expected import path segment");
                break;
            }
            parts.push_back(currentTokenIndex(p));
            advance(p);
        }
        return parts;
    }

    [[nodiscard]] inline u32 makeImportPathFromParts(Parser& p, const vector<u32>& parts) noexcept {
        vector<u32> children;
        children.reserve(parts.size());
        for (const u32 tokenIndex : parts) children.push_back(makeLeaf(p, NodeKind::ImportSegment, tokenIndex));
        const u32 tokIdx = parts.empty() ? currentTokenIndex(p) : parts.front();
        return makeNode(p, NodeKind::ImportPath, tokIdx, children);
    }

    inline void parseImportGroup(Parser& p, const vector<u32>& prefix, vector<u32>& children) noexcept;

    inline void parseImportEntry(Parser& p, const vector<u32>& prefix, vector<u32>& children) noexcept {
        bool hasGroupedItems = false;
        const vector<u32> suffix = parseImportPathParts(p, hasGroupedItems);
        vector<u32> fullPath = prefix;
        fullPath.insert(fullPath.end(), suffix.begin(), suffix.end());
        if (hasGroupedItems) {
            parseImportGroup(p, fullPath, children);
            return;
        }
        const u32 path = makeImportPathFromParts(p, fullPath);
        const u32 itemIdx = fullPath.empty() ? currentTokenIndex(p) : fullPath.back();
        children.push_back(makeNode(p, NodeKind::ImportItem, itemIdx, vector<u32>{path}));
    }

    inline void parseImportGroup(Parser& p, const vector<u32>& prefix, vector<u32>& children) noexcept {
        expect(p, TokenType::CH_LEFT_BRACE, ErrorKind::UnexpectedToken, "expected '{' to start import item list");
        while (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) {
            parseImportEntry(p, prefix, children);
            if (match(p, TokenType::CH_COMMA)) continue;
            break;
        }
        expect(p, TokenType::CH_RIGHT_BRACE, ErrorKind::ExpectedRightBrace);
    }

    [[nodiscard]] inline GenericParseResult parseGenericParams(Parser& p) noexcept {
        GenericParseResult result;
        if (!match(p, TokenType::OP_LESS)) return result;
        while (!check(p, TokenType::OP_GREATER) && !atEnd(p)) {
            u64 kind = 0;
            if (match(p, TokenType::KW_VAL)) kind = 1;
            else if (match(p, TokenType::KW_TRAIT)) kind = 2;
            else (void)match(p, TokenType::KW_TYPE);

            const u32 nameIdx = currentTokenIndex(p);
            if (!expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier, "expected generic parameter name")) {
                result.nodes.push_back(makeErrorNode(p));
                break;
            }

            vector<u32> children;
            if (match(p, TokenType::OP_EQUAL)) children.push_back(parseType(p));
            result.nodes.push_back(makeNode(p, NodeKind::GenericParam, nameIdx, children, 0, kind));
            result.count++;
            if (match(p, TokenType::CH_COMMA)) continue;
            break;
        }
        expect(p, TokenType::OP_GREATER, ErrorKind::ExpectedRightAngle);
        return result;
    }

    [[nodiscard]] inline u32 parseParam(Parser& p) noexcept {
        const u64 saved = p.cursor;
        u64 selfFlags = 0;
        if (peekType(p) == TokenType::OP_TILDE && peekTypeAhead(p, 1) == TokenType::KW_SELF) {
            selfFlags |= 2;
            advance(p);
        }
        if (peekType(p) == TokenType::KW_SELF) {
            const u32 tokIdx = currentTokenIndex(p);
            advance(p);
            if (match(p, TokenType::OP_BANG)) selfFlags |= 1;
            return makeLeaf(p, NodeKind::SelfParam, tokIdx, 0, selfFlags);
        }
        p.cursor = saved;

        const u32 typeNode = parseType(p);
        const u32 nameIdx = currentTokenIndex(p);
        expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier, "expected parameter name");
        return makeNode(p, NodeKind::Param, nameIdx, vector<u32>{typeNode});
    }

    [[nodiscard]] inline vector<u32> parseParams(Parser& p) noexcept {
        vector<u32> params;
        expect(p, TokenType::CH_LEFT_PAREN, ErrorKind::UnexpectedToken, "expected '(' to start parameter list");
        if (match(p, TokenType::CH_RIGHT_PAREN)) return params;
        while (!check(p, TokenType::CH_RIGHT_PAREN) && !atEnd(p)) {
            params.push_back(parseParam(p));
            if (match(p, TokenType::CH_COMMA)) continue;
            break;
        }
        expect(p, TokenType::CH_RIGHT_PAREN, ErrorKind::ExpectedRightParen);
        return params;
    }

    [[nodiscard]] inline BodyParseResult parseBody(Parser& p) noexcept {
        BodyParseResult result;
        if (match(p, TokenType::OP_EQUAL_GREATER)) {
            const u32 tokIdx = currentTokenIndex(p);
            const u32 expr = parseExpression(p);
            result.node = makeNode(p, NodeKind::ExprStmt, tokIdx, vector<u32>{expr});
            result.payloadFlags = FN_PAYLOAD_EXPR_BODY;
            if (!match(p, TokenType::CH_SEMICOLON) && !check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) {
                recordError(p, ErrorKind::ExpectedSemicolon);
                resyncToStatementBoundary(p);
            }
            return result;
        }
        if (check(p, TokenType::CH_LEFT_BRACE)) {
            result.node = parseBlock(p);
            return result;
        }
        if (match(p, TokenType::CH_SEMICOLON)) {
            result.node = makeLeaf(p, NodeKind::Block, currentTokenIndex(p), FLAG_SYNTHETIC);
            return result;
        }
        recordError(p, ErrorKind::UnexpectedToken, "expected function body");
        result.node = makeErrorNode(p);
        return result;
    }

    [[nodiscard]] inline u32 parseOperatorName(Parser& p, u64& opPayload) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        if (!isOperatorNameToken(peekType(p))) {
            recordError(p, ErrorKind::ExpectedIdentifier, "expected operator name");
            return tokIdx;
        }
        opPayload = static_cast<u64>(peekType(p));
        advance(p);
        while (isOperatorNameToken(peekType(p)) && peekType(p) != TokenType::LT_IDENTIFIER && peekType(p) != TokenType::CH_LEFT_PAREN) advance(p);
        return tokIdx;
    }

    [[nodiscard]] inline u32 makeCallable(Parser& p, NodeKind kind, u16 flags, u32 tokIdx, u32 returnType, GenericParseResult generic, u64 opPayload = 0) noexcept {
        vector<u32> children;
        children.reserve(generic.nodes.size() + 4);
        children.push_back(returnType);
        for (u32 node : generic.nodes) children.push_back(node);
        const vector<u32> params = parseParams(p);
        for (u32 param : params) children.push_back(param);
        const BodyParseResult body = parseBody(p);
        children.push_back(body.node);
        u64 payload = body.payloadFlags;
        if (kind == NodeKind::OperatorDecl) payload |= opPayload << 8;
        return makeNode(p, kind, tokIdx, children, flags, payload);
    }

    [[nodiscard]] inline u32 parseOperatorAfterReturn(Parser& p, u16 flags, u32 returnType) noexcept {
        advance(p);
        u64 opPayload = 0;
        const u32 opIdx = parseOperatorName(p, opPayload);
        GenericParseResult generic = parseGenericParams(p);
        return makeCallable(p, NodeKind::OperatorDecl, flags, opIdx, returnType, generic, opPayload);
    }

    [[nodiscard]] inline u32 parseConversionOperator(Parser& p, u16 flags) noexcept {
        const u32 opIdx = currentTokenIndex(p);
        advance(p);
        const u32 returnType = parseType(p);
        GenericParseResult generic = parseGenericParams(p);
        return makeCallable(p, NodeKind::OperatorDecl, flags, opIdx, returnType, generic);
    }

    [[nodiscard]] inline u32 parseDeclarationAfterType(Parser& p, u16 flags, u32 typeNode, u32 nameIdx, bool memberScope) noexcept {
        GenericParseResult generic = parseGenericParams(p);
        if (check(p, TokenType::CH_LEFT_PAREN)) return makeCallable(p, NodeKind::FunctionDecl, flags, nameIdx, typeNode, generic);

        vector<u32> children;
        children.push_back(typeNode);
        if (match(p, TokenType::OP_EQUAL)) children.push_back(parseExpression(p));
        if (!match(p, TokenType::CH_SEMICOLON)) {
            recordError(p, ErrorKind::ExpectedSemicolon);
            resyncToStatementBoundary(p);
        }
        const NodeKind kind = memberScope ? NodeKind::FieldDecl : NodeKind::VariableDecl;
        return makeNode(p, kind, nameIdx, children, flags);
    }

    [[nodiscard]] inline u32 parseDupDecl(Parser& p, u16 flags) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        vector<u32> children;
        if (match(p, TokenType::OP_EQUAL)) children.push_back(parseExpression(p));
        if (!match(p, TokenType::CH_SEMICOLON)) {
            recordError(p, ErrorKind::ExpectedSemicolon);
            resyncToStatementBoundary(p);
        }
        return makeNode(p, NodeKind::DupDecl, tokIdx, children, flags);
    }

    [[nodiscard]] inline u32 parseModuleDecl(Parser& p, u16 flags) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        const u32 path = parsePath(p);
        expect(p, TokenType::CH_SEMICOLON, ErrorKind::ExpectedSemicolon);
        return makeNode(p, NodeKind::ModuleDecl, tokIdx, vector<u32>{path}, flags);
    }

    [[nodiscard]] inline u32 parseImportDecl(Parser& p, u16 flags) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        vector<u32> children;
        while (!check(p, TokenType::CH_SEMICOLON) && !atEnd(p)) {
            parseImportEntry(p, vector<u32>{}, children);
            if (match(p, TokenType::CH_COMMA)) continue;
            break;
        }
        expect(p, TokenType::CH_SEMICOLON, ErrorKind::ExpectedSemicolon);
        return makeNode(p, NodeKind::ImportDecl, tokIdx, children, flags);
    }

    [[nodiscard]] inline u32 parseTypeDecl(Parser& p, u16 flags) noexcept {
        advance(p);
        const u32 nameIdx = currentTokenIndex(p);
        expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier, "expected type name");
        GenericParseResult generic = parseGenericParams(p);
        vector<u32> children = generic.nodes;
        expect(p, TokenType::CH_LEFT_BRACE, ErrorKind::UnexpectedToken, "expected '{' to start type body");
        while (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) children.push_back(parseItem(p, 0, true));
        expect(p, TokenType::CH_RIGHT_BRACE, ErrorKind::ExpectedRightBrace);
        return makeNode(p, NodeKind::TypeDecl, nameIdx, children, flags, generic.count);
    }

    [[nodiscard]] inline u32 parseTraitDecl(Parser& p, u16 flags) noexcept {
        advance(p);
        const u32 nameIdx = currentTokenIndex(p);
        expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier, "expected trait name");
        GenericParseResult generic = parseGenericParams(p);
        vector<u32> children = generic.nodes;
        if (match(p, TokenType::KW_IF)) children.push_back(parseHeadExpression(p));
        expect(p, TokenType::CH_LEFT_BRACE, ErrorKind::UnexpectedToken, "expected '{' to start trait body");
        while (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) children.push_back(parseItem(p, 0, true));
        expect(p, TokenType::CH_RIGHT_BRACE, ErrorKind::ExpectedRightBrace);
        return makeNode(p, NodeKind::TraitDecl, nameIdx, children, flags, generic.count);
    }

    [[nodiscard]] inline u32 parseEnumDecl(Parser& p, u16 flags) noexcept {
        advance(p);
        const u32 nameIdx = currentTokenIndex(p);
        expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier, "expected enum name");
        vector<u32> children;
        expect(p, TokenType::CH_LEFT_BRACE, ErrorKind::UnexpectedToken, "expected '{' to start enum body");
        while (!check(p, TokenType::CH_RIGHT_BRACE) && !atEnd(p)) {
            const u32 variantIdx = currentTokenIndex(p);
            if (!expect(p, TokenType::LT_IDENTIFIER, ErrorKind::ExpectedIdentifier, "expected enum variant")) break;
            vector<u32> payload;
            if (check(p, TokenType::CH_LEFT_PAREN)) {
                advance(p);
                if (!check(p, TokenType::CH_RIGHT_PAREN)) payload.push_back(parseType(p));
                expect(p, TokenType::CH_RIGHT_PAREN, ErrorKind::ExpectedRightParen);
            }
            children.push_back(makeNode(p, NodeKind::EnumVariant, variantIdx, payload));
            if (match(p, TokenType::CH_COMMA) || match(p, TokenType::CH_SEMICOLON)) continue;
            break;
        }
        expect(p, TokenType::CH_RIGHT_BRACE, ErrorKind::ExpectedRightBrace);
        return makeNode(p, NodeKind::EnumDecl, nameIdx, children, flags);
    }

    [[nodiscard]] inline u32 parseVisibilityLabel(Parser& p, u16 flags, TokenType keyword) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        advance(p);
        expect(p, TokenType::OP_COLON, ErrorKind::UnexpectedToken, "expected ':' after visibility label");
        u16 labelFlags = flags;
        if (keyword == TokenType::KW_PUBLIC) labelFlags |= FLAG_PUBLIC;
        else labelFlags |= FLAG_PRIVATE;
        return makeLeaf(p, NodeKind::VisibilityLabel, tokIdx, labelFlags);
    }

    [[nodiscard]] inline u32 parseTypedOrCallableItem(Parser& p, u16 flags, bool memberScope) noexcept {
        const u64 saved = p.cursor;
        if (!tryParseType(p)) {
            p.cursor = saved;
            recordError(p, ErrorKind::UnknownDeclaration);
            const u32 err = makeErrorNode(p);
            resyncToStatementBoundary(p);
            return err;
        }
        if (peekType(p) != TokenType::LT_IDENTIFIER && peekType(p) != TokenType::KW_OP) {
            p.cursor = saved;
            recordError(p, ErrorKind::UnknownDeclaration);
            const u32 err = makeErrorNode(p);
            resyncToStatementBoundary(p);
            return err;
        }
        p.cursor = saved;
        const u32 typeNode = parseType(p);
        if (check(p, TokenType::KW_OP)) return parseOperatorAfterReturn(p, flags, typeNode);
        const u32 nameIdx = currentTokenIndex(p);
        advance(p);
        return parseDeclarationAfterType(p, flags, typeNode, nameIdx, memberScope);
    }

    [[nodiscard]] inline u32 parseItem(Parser& p, u16 flags, bool memberScope) noexcept {
        skipTrivia(p);
        for (;;) {
            const TokenType t = peekType(p);
            if (t == TokenType::KW_EXPORT) {
                flags |= FLAG_EXPORT;
                advance(p);
                continue;
            }
            if (t == TokenType::KW_PUBLIC || t == TokenType::KW_PRIVATE) {
                if (peekTypeAhead(p, 1) == TokenType::OP_COLON) return parseVisibilityLabel(p, flags, t);
                flags |= t == TokenType::KW_PUBLIC ? FLAG_PUBLIC : FLAG_PRIVATE;
                advance(p);
                continue;
            }
            break;
        }

        switch (peekType(p)) {
            case TokenType::KW_MODULE: return parseModuleDecl(p, flags);
            case TokenType::KW_IMPORT: return parseImportDecl(p, flags);
            case TokenType::KW_TYPE: return parseTypeDecl(p, flags);
            case TokenType::KW_TRAIT: return parseTraitDecl(p, flags);
            case TokenType::KW_ENUM: return parseEnumDecl(p, flags);
            case TokenType::KW_DUP: return parseDupDecl(p, flags);
            case TokenType::KW_OP: return parseConversionOperator(p, flags);
            case TokenType::MISC_EOF: return makeErrorNode(p);
            default: return parseTypedOrCallableItem(p, flags, memberScope);
        }
    }
}