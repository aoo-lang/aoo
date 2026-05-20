#pragma once
#include <array>
#include <cstddef>
#include <initializer_list>
#include <vector>

#include "../lexer/tokens.hpp"
#include "ASTNodes.hpp"
#include "Errors.hpp"
#include "Speculation.hpp"
#include "State.hpp"

namespace AOO::Parser {
    using std::array, Lexer::TokenType;

    //Forward declaration — defined further down.
    [[nodiscard]] inline u32 parseExpr(Parser& p, int rbp, ParseMode mode) noexcept;

    //Type-mode entry point. Wraps parseExpr in type mode.
    [[nodiscard]] inline u32 parseType(Parser& p) noexcept { return parseExpr(p, 0, ParseMode::Type); }

    //Top-level expression entry (right binding power 0).
    [[nodiscard]] inline u32 parseExpression(Parser& p) noexcept { return parseExpr(p, 0, ParseMode::Value); }

    //Convenience: parse a comma-separated list of expressions until the given closer.
    //Caller is responsible for consuming the opening token; this consumes the closing token.
    [[nodiscard]] inline vector<u32> parseExprList(Parser& p, TokenType closer, ParseMode mode) noexcept {
        vector<u32> out;
        if (peekType(p) == closer) { advance(p); return out; }
        for (;;) {
            out.push_back(parseExpr(p, 0, mode));
            if (peekType(p) == TokenType::CH_COMMA) { advance(p); continue; }
            break;
        }
        if (!expect(p, closer, ErrorKind::UnexpectedToken)) {
            //Resync: just stop. Closer not present, leave cursor; caller may diagnose.
        }
        return out;
    }

    //-------------------- precedence constants --------------------

    constexpr int LBP_NONE       = 0;
    constexpr int LBP_ASSIGN     = 30;   //tier 3, right-assoc
    constexpr int LBP_TERNARY    = 40;   //tier 4, right-assoc
    constexpr int LBP_RANGE      = 50;   //tier 5, non-chain
    constexpr int LBP_OR         = 60;   //tier 6, ||
    constexpr int LBP_AND        = 70;   //tier 7, &&
    constexpr int LBP_COMPARE    = 80;   //tier 8, non-chain
    constexpr int LBP_BIT_OR     = 90;
    constexpr int LBP_BIT_XOR    = 100;
    constexpr int LBP_BIT_AND    = 110;
    constexpr int LBP_SHIFT      = 120;
    constexpr int LBP_ADD        = 130;
    constexpr int LBP_MUL        = 140;
    constexpr int LBP_AS         = 150;
    constexpr int LBP_PREFIX_DUP = 155;  //tier 15.5
    constexpr int LBP_PREFIX     = 160;
    constexpr int LBP_POSTFIX    = 170;

    //OP_LESS may be: generic-app (170), shift-assign (30), shift (120), <= or < (80).
    //Use the maximum so the loop calls the handler whenever it could plausibly fire.
    constexpr int LBP_OP_LESS    = LBP_POSTFIX;
    //OP_GREATER may be: shift-assign (30), shift (120), >= or > (80).
    constexpr int LBP_OP_GREATER = LBP_SHIFT;

    //-------------------- helpers --------------------

    //Build a binary-op node.
    [[nodiscard]] inline u32 makeBinaryOp(Parser& p, u32 left, u32 right, u32 opTokenIdx, TokenType opType) noexcept {
        const u32 firstChild = reserveChildren(p.ast, 2);
        p.ast.childIndices[firstChild + 0] = left;
        p.ast.childIndices[firstChild + 1] = right;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::BinaryOp, .flags = 0, .tokenIndex = opTokenIdx,
            .firstChild = firstChild, .childCount = 2, .payload = static_cast<u64>(opType)});
    }

    [[nodiscard]] inline u32 makeUnaryPrefix(Parser& p, u32 operand, u32 opTokenIdx, TokenType opType) noexcept {
        const u32 firstChild = reserveChildren(p.ast, 1);
        p.ast.childIndices[firstChild] = operand;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::UnaryPrefix, .flags = 0, .tokenIndex = opTokenIdx,
            .firstChild = firstChild, .childCount = 1, .payload = static_cast<u64>(opType)});
    }

    [[nodiscard]] inline u32 makeUnaryPostfix(Parser& p, u32 operand, u32 opTokenIdx, TokenType opType) noexcept {
        const u32 firstChild = reserveChildren(p.ast, 1);
        p.ast.childIndices[firstChild] = operand;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::UnaryPostfix, .flags = 0, .tokenIndex = opTokenIdx,
            .firstChild = firstChild, .childCount = 1, .payload = static_cast<u64>(opType)});
    }

    [[nodiscard]] inline bool isComparison(TokenType t) noexcept {
        switch (t) {
            case TokenType::OP_DOUBLE_EQUAL:
            case TokenType::OP_BANG_EQUAL:
            case TokenType::OP_LESS:
            case TokenType::OP_LESS_EQUAL:
            case TokenType::OP_GREATER:
            case TokenType::OP_GREATER_EQUAL:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] inline bool isRange(TokenType t) noexcept {
        return t == TokenType::OP_DOUBLE_PERIOD || t == TokenType::OP_TRIPLE_PERIOD;
    }

    [[nodiscard]] inline bool isConditionStructBase(ASTNode const& node) noexcept {
        switch (node.kind) {
            case NodeKind::GenericApp:
            case NodeKind::ScopeAccess:
            case NodeKind::DoubleScopeAccess:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] inline bool braceOpensConditionStructLit(Parser& p, u32 left) noexcept {
        if (left >= p.ast.nodes.size()) return false;
        if (!isConditionStructBase(p.ast.nodes[left])) return false;
        const TokenType first = peekTypeAhead(p, 1);
        if (first == TokenType::CH_RIGHT_BRACE) return true;
        if (first != TokenType::LT_IDENTIFIER) return false;
        const TokenType second = peekTypeAhead(p, 2);
        if (second == TokenType::OP_EQUAL) return true;
        if (second == TokenType::OP_COLON && peekTypeAhead(p, 3) == TokenType::OP_EQUAL) return true;
        return false;
    }

    [[nodiscard]] inline bool isIntLiteral(TokenType t) noexcept {
        using enum TokenType;
        switch (t) {
            case LT_BINARY_INT:
            case LT_BINARY_INT_U8: case LT_BINARY_INT_U16: case LT_BINARY_INT_U32: case LT_BINARY_INT_U64:
            case LT_BINARY_INT_I8: case LT_BINARY_INT_I16: case LT_BINARY_INT_I32: case LT_BINARY_INT_I64:
            case LT_OCTAL_INT:
            case LT_OCTAL_INT_U8: case LT_OCTAL_INT_U16: case LT_OCTAL_INT_U32: case LT_OCTAL_INT_U64:
            case LT_OCTAL_INT_I8: case LT_OCTAL_INT_I16: case LT_OCTAL_INT_I32: case LT_OCTAL_INT_I64:
            case LT_DECIMAL_INT:
            case LT_DECIMAL_INT_U8: case LT_DECIMAL_INT_U16: case LT_DECIMAL_INT_U32: case LT_DECIMAL_INT_U64:
            case LT_DECIMAL_INT_I8: case LT_DECIMAL_INT_I16: case LT_DECIMAL_INT_I32: case LT_DECIMAL_INT_I64:
            case LT_HEX_INT:
            case LT_HEX_INT_U8: case LT_HEX_INT_U16: case LT_HEX_INT_U32: case LT_HEX_INT_U64:
            case LT_HEX_INT_I8: case LT_HEX_INT_I16: case LT_HEX_INT_I32: case LT_HEX_INT_I64:
                return true;
            default: return false;
        }
    }

    [[nodiscard]] inline bool isFloatLiteral(TokenType t) noexcept {
        using enum TokenType;
        return t == LT_DECIMAL_FLOAT || t == LT_DECIMAL_FLOAT_F32 || t == LT_DECIMAL_FLOAT_F64
            || t == LT_HEX_FLOAT     || t == LT_HEX_FLOAT_F32     || t == LT_HEX_FLOAT_F64;
    }

    //-------------------- prefix handlers --------------------

    [[nodiscard]] inline u32 prefIdentifier(Parser& p, ParseMode mode) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = mode == ParseMode::Type ? NodeKind::TypeIdent : NodeKind::Identifier,
            .flags = 0, .tokenIndex = idx, .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefIntLiteral(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::IntLit, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefFloatLiteral(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::FloatLit, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefStringLiteral(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::StringLit, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefCharLiteral(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        const u64 ch = peek(p).charPayload;
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::CharLit, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = ch});
    }

    [[nodiscard]] inline u32 prefLabelLiteral(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::LabelLit, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefSelf(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::SelfExpr, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefVoid(Parser& p, ParseMode mode) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = mode == ParseMode::Type ? NodeKind::TypeVoid : NodeKind::VoidExpr,
            .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefAuto(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::TypeAuto, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefNoReturn(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::TypeNoReturn, .flags = 0, .tokenIndex = idx,
            .firstChild = 0, .childCount = 0, .payload = 0});
    }

    [[nodiscard]] inline u32 prefParen(Parser& p, ParseMode mode) noexcept {
        advance(p); //consume (
        const u32 inner = parseExpr(p, 0, mode);
        if (!expect(p, TokenType::CH_RIGHT_PAREN, ErrorKind::ExpectedRightParen)) {
            //Best-effort: keep going.
        }
        return inner;
    }

    [[nodiscard]] inline u32 prefUnary(Parser& p, ParseMode mode) noexcept {
        const u32 idx = currentTokenIndex(p);
        const TokenType opType = peekType(p);
        //Reject prefix !! per spec.
        if (opType == TokenType::OP_DOUBLE_BANG) {
            recordError(p, ErrorKind::BangBangPrefix, "'!!' is postfix-only; did you mean '!(!a)'?");
            return makeErrorNode(p);
        }
        if (opType == TokenType::OP_DOUBLE_PLUS || opType == TokenType::OP_DOUBLE_DASH) {
            recordError(p, ErrorKind::IncDecPrefix, "'++'/'--' are postfix-only");
            return makeErrorNode(p);
        }
        advance(p);
        const u32 operand = parseExpr(p, LBP_PREFIX, mode);
        //For `~` in type-mode, build TypeConsumed. Otherwise generic UnaryPrefix.
        if (mode == ParseMode::Type && opType == TokenType::OP_TILDE) {
            const u32 firstChild = reserveChildren(p.ast, 1);
            p.ast.childIndices[firstChild] = operand;
            return addNode(p.ast, ASTNode{
                .kind = NodeKind::TypeConsumed, .flags = 0, .tokenIndex = idx,
                .firstChild = firstChild, .childCount = 1, .payload = 0});
        }
        return makeUnaryPrefix(p, operand, idx, opType);
    }

    [[nodiscard]] inline u32 prefDup(Parser& p, ParseMode /*mode*/) noexcept {
        const u32 idx = currentTokenIndex(p);
        advance(p);
        const u32 operand = parseExpr(p, LBP_PREFIX_DUP, ParseMode::Value);
        const u32 firstChild = reserveChildren(p.ast, 1);
        p.ast.childIndices[firstChild] = operand;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::Dup, .flags = 0, .tokenIndex = idx,
            .firstChild = firstChild, .childCount = 1, .payload = 0});
    }

    //-------------------- infix handlers --------------------

    //Build a binary op consuming a single token.
    [[nodiscard]] inline u32 inSimpleBinary(Parser& p, u32 left, int rbp, ParseMode mode, int lbp, bool rightAssoc, bool nonChain, ErrorKind chainErr) noexcept {
        if (lbp <= rbp) return left;
        const u32 opIdx = currentTokenIndex(p);
        const TokenType opType = peekType(p);
        advance(p);
        const int recurseRbp = rightAssoc ? lbp - 1 : lbp;
        const u32 right = parseExpr(p, recurseRbp, mode);
        const u32 node = makeBinaryOp(p, left, right, opIdx, opType);
        if (nonChain) {
            const TokenType next = peekType(p);
            if ((chainErr == ErrorKind::ComparisonChained && isComparison(next))
                || (chainErr == ErrorKind::RangeChained && isRange(next))) {
                recordError(p, chainErr);
            }
        }
        return node;
    }

    [[nodiscard]] inline u32 inAdd(Parser& p, u32 l, int rbp, ParseMode m) noexcept    { return inSimpleBinary(p, l, rbp, m, LBP_ADD, false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inMul(Parser& p, u32 l, int rbp, ParseMode m) noexcept    { return inSimpleBinary(p, l, rbp, m, LBP_MUL, false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inAnd(Parser& p, u32 l, int rbp, ParseMode m) noexcept    { return inSimpleBinary(p, l, rbp, m, LBP_AND, false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inOr(Parser& p, u32 l, int rbp, ParseMode m) noexcept     { return inSimpleBinary(p, l, rbp, m, LBP_OR,  false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inBitAnd(Parser& p, u32 l, int rbp, ParseMode m) noexcept { return inSimpleBinary(p, l, rbp, m, LBP_BIT_AND, false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inBitOr(Parser& p, u32 l, int rbp, ParseMode m) noexcept  { return inSimpleBinary(p, l, rbp, m, LBP_BIT_OR,  false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inBitXor(Parser& p, u32 l, int rbp, ParseMode m) noexcept { return inSimpleBinary(p, l, rbp, m, LBP_BIT_XOR, false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inShift(Parser& p, u32 l, int rbp, ParseMode m) noexcept  { return inSimpleBinary(p, l, rbp, m, LBP_SHIFT, false, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inEqNeq(Parser& p, u32 l, int rbp, ParseMode m) noexcept  { return inSimpleBinary(p, l, rbp, m, LBP_COMPARE, false, true, ErrorKind::ComparisonChained); }
    [[nodiscard]] inline u32 inAssign(Parser& p, u32 l, int rbp, ParseMode m) noexcept { return inSimpleBinary(p, l, rbp, m, LBP_ASSIGN, true, false, ErrorKind::InternalBug); }
    [[nodiscard]] inline u32 inRange(Parser& p, u32 l, int rbp, ParseMode m) noexcept {
        if (LBP_RANGE <= rbp) return l;
        const u32 opIdx = currentTokenIndex(p);
        const TokenType opType = peekType(p);
        advance(p);
        const u32 right = parseExpr(p, LBP_RANGE, m);
        const u32 firstChild = reserveChildren(p.ast, 2);
        p.ast.childIndices[firstChild + 0] = l;
        p.ast.childIndices[firstChild + 1] = right;
        const u32 node = addNode(p.ast, ASTNode{
            .kind = NodeKind::Range, .flags = 0, .tokenIndex = opIdx,
            .firstChild = firstChild, .childCount = 2, .payload = static_cast<u64>(opType)});
        if (isRange(peekType(p))) recordError(p, ErrorKind::RangeChained);
        return node;
    }

    //`as` infix cast — RHS is type-mode.
    [[nodiscard]] inline u32 inAs(Parser& p, u32 left, int rbp, ParseMode /*mode*/) noexcept {
        if (LBP_AS <= rbp) return left;
        const u32 opIdx = currentTokenIndex(p);
        advance(p);
        const u32 type = parseExpr(p, LBP_AS, ParseMode::Type);
        const u32 firstChild = reserveChildren(p.ast, 2);
        p.ast.childIndices[firstChild + 0] = left;
        p.ast.childIndices[firstChild + 1] = type;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::AsCast, .flags = 0, .tokenIndex = opIdx,
            .firstChild = firstChild, .childCount = 2, .payload = 0});
    }

    //a ?: b :: c   (ternary)
    [[nodiscard]] inline u32 inTernaryQ(Parser& p, u32 cond, int rbp, ParseMode mode) noexcept {
        if (LBP_TERNARY <= rbp) return cond;
        const u32 opIdx = currentTokenIndex(p);
        advance(p); //consume ?:
        const u32 thenE = parseExpr(p, LBP_TERNARY - 1, mode);
        if (!expect(p, TokenType::OP_DOUBLE_COLON, ErrorKind::UnexpectedToken, "expected '::' to close ternary")) {
            //Continue best-effort.
        }
        const u32 elseE = parseExpr(p, LBP_TERNARY - 1, mode);
        const u32 firstChild = reserveChildren(p.ast, 3);
        p.ast.childIndices[firstChild + 0] = cond;
        p.ast.childIndices[firstChild + 1] = thenE;
        p.ast.childIndices[firstChild + 2] = elseE;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::TernaryQ, .flags = 0, .tokenIndex = opIdx,
            .firstChild = firstChild, .childCount = 3, .payload = 0});
    }

    //Postfix: ! !! ++ --
    [[nodiscard]] inline u32 inPostfixUnary(Parser& p, u32 left, int rbp, ParseMode mode) noexcept {
        if (LBP_POSTFIX <= rbp) return left;
        const u32 opIdx = currentTokenIndex(p);
        const TokenType opType = peekType(p);
        advance(p);
        //In type-mode, postfix `!` `!!` `*` `&` build typed wrappers.
        if (mode == ParseMode::Type) {
            NodeKind k;
            switch (opType) {
                case TokenType::OP_BANG:        k = NodeKind::TypeMut; break;
                case TokenType::OP_DOUBLE_BANG: k = NodeKind::TypeMustInit; break;
                case TokenType::OP_STAR:        k = NodeKind::TypePtr; break;
                case TokenType::OP_AMPERSAND:   k = NodeKind::TypeRef; break;
                default:                        k = NodeKind::Error; break;
            }
            const u32 firstChild = reserveChildren(p.ast, 1);
            p.ast.childIndices[firstChild] = left;
            return addNode(p.ast, ASTNode{
                .kind = k, .flags = 0, .tokenIndex = opIdx,
                .firstChild = firstChild, .childCount = 1, .payload = static_cast<u64>(opType)});
        }
        return makeUnaryPostfix(p, left, opIdx, opType);
    }

    //Member access: . -> :
    [[nodiscard]] inline u32 inMemberAccess(Parser& p, u32 left, int rbp, ParseMode /*mode*/) noexcept {
        const TokenType opType = peekType(p);
        const u32 opIdx = currentTokenIndex(p);
        if (opType == TokenType::OP_COLON && peekTypeAhead(p, 1) == TokenType::OP_EQUAL) {
            if (LBP_ASSIGN <= rbp) return left;
            advance(p);
            advance(p);
            const u32 right = parseExpr(p, LBP_ASSIGN - 1, ParseMode::Value);
            return makeBinaryOp(p, left, right, opIdx, TokenType::OP_COLON);
        }
        if (LBP_POSTFIX <= rbp) return left;
        advance(p); //consume the access op
        const u32 nameIdx = currentTokenIndex(p);
        if (peekType(p) != TokenType::LT_IDENTIFIER) {
            recordError(p, ErrorKind::ExpectedIdentifier, "expected identifier after access op");
            return left;
        }
        advance(p);
        NodeKind k;
        switch (opType) {
            case TokenType::OP_PERIOD:        k = NodeKind::MemberAccess; break;
            case TokenType::OP_DASH_GREATER:  k = NodeKind::ArrowAccess;  break;
            case TokenType::OP_COLON:         k = NodeKind::ScopeAccess;  break;
            case TokenType::OP_DOUBLE_COLON:  k = NodeKind::DoubleScopeAccess; break;
            default:                          k = NodeKind::Error; break;
        }
        const u32 firstChild = reserveChildren(p.ast, 1);
        p.ast.childIndices[firstChild] = left;
        return addNode(p.ast, ASTNode{
            .kind = k, .flags = 0, .tokenIndex = nameIdx,
            .firstChild = firstChild, .childCount = 1, .payload = 0});
    }

    //Call: callee ( args )
    [[nodiscard]] inline u32 inCall(Parser& p, u32 callee, int rbp, ParseMode /*mode*/) noexcept {
        if (LBP_POSTFIX <= rbp) return callee;
        const u32 opIdx = currentTokenIndex(p);
        advance(p); //(
        const auto args = parseExprList(p, TokenType::CH_RIGHT_PAREN, ParseMode::Value);
        vector<u32> kids;
        kids.reserve(args.size() + 1);
        kids.push_back(callee);
        for (u32 a : args) kids.push_back(a);
        const u32 firstChild = appendChildren(p.ast, kids);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::Call, .flags = 0, .tokenIndex = opIdx,
            .firstChild = firstChild, .childCount = static_cast<u32>(kids.size()), .payload = 0});
    }

    //Index: target [ idx ]
    [[nodiscard]] inline u32 inIndex(Parser& p, u32 target, int rbp, ParseMode mode) noexcept {
        if (LBP_POSTFIX <= rbp) return target;
        const u32 opIdx = currentTokenIndex(p);
        advance(p); //[
        const u32 idxExpr = parseExpr(p, 0, mode);
        if (!expect(p, TokenType::CH_RIGHT_BRACKET, ErrorKind::ExpectedRightBracket)) { /* best-effort */ }
        const u32 firstChild = reserveChildren(p.ast, 2);
        p.ast.childIndices[firstChild + 0] = target;
        p.ast.childIndices[firstChild + 1] = idxExpr;
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::Index, .flags = 0, .tokenIndex = opIdx,
            .firstChild = firstChild, .childCount = 2, .payload = 0});
    }

    //Struct literal: type { field_inits... }
    //Each field is one of:
    //  IDENT  '='  expr          → assign (payload=1)
    //  IDENT  ':=' expr          → copy-init (payload=2) -- written as IDENT then OP_COLON then OP_EQUAL
    //  IDENT                     → pun (payload=0)
    [[nodiscard]] inline u32 inStructLit(Parser& p, u32 typeExpr, int rbp, ParseMode /*mode*/) noexcept {
        if (LBP_POSTFIX <= rbp) return typeExpr;
        const u32 opIdx = currentTokenIndex(p);
        advance(p); //{
        vector<u32> fields;
        fields.push_back(typeExpr);
        while (peekType(p) != TokenType::CH_RIGHT_BRACE && !atEnd(p)) {
            const u32 nameIdx = currentTokenIndex(p);
            if (peekType(p) != TokenType::LT_IDENTIFIER) {
                recordError(p, ErrorKind::ExpectedIdentifier, "expected field name in struct literal");
                if (peekType(p) == TokenType::CH_COMMA) { advance(p); continue; }
                break;
            }
            advance(p);
            u64 initKind = 0; //pun
            u32 valueIdx = 0;
            u32 childCount = 0;
            if (peekType(p) == TokenType::OP_EQUAL) {
                advance(p);
                initKind = 1;
                valueIdx = parseExpr(p, 0, ParseMode::Value);
                childCount = 1;
            } else if (peekType(p) == TokenType::OP_COLON) {
                //Could be `:=` (copy-init). Lexer never emits a fused `:=`, so check OP_EQUAL after.
                advance(p);
                if (!expect(p, TokenType::OP_EQUAL, ErrorKind::UnexpectedToken, "expected '=' to form ':='")) {
                    //fall through anyway
                }
                initKind = 2;
                valueIdx = parseExpr(p, 0, ParseMode::Value);
                childCount = 1;
            }
            const u32 firstChild = childCount ? reserveChildren(p.ast, 1) : 0;
            if (childCount) p.ast.childIndices[firstChild] = valueIdx;
            const u32 fieldNode = addNode(p.ast, ASTNode{
                .kind = NodeKind::StructLitField, .flags = 0, .tokenIndex = nameIdx,
                .firstChild = firstChild, .childCount = childCount, .payload = initKind});
            fields.push_back(fieldNode);
            if (peekType(p) == TokenType::CH_COMMA) { advance(p); continue; }
            if (peekType(p) == TokenType::CH_SEMICOLON) { advance(p); continue; }
            break;
        }
        if (!expect(p, TokenType::CH_RIGHT_BRACE, ErrorKind::ExpectedRightBrace)) { /* best-effort */ }
        const u32 firstChild = appendChildren(p.ast, fields);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::StructLit, .flags = 0, .tokenIndex = opIdx,
            .firstChild = firstChild, .childCount = static_cast<u32>(fields.size()), .payload = 0});
    }

    //Generic application: build node from already-known type-arg list.
    [[nodiscard]] inline u32 buildGenericApp(Parser& p, u32 base, u32 ltTokenIdx) noexcept {
        //Cursor must be just past the opening '<'. Parse type-args until matching '>'.
        vector<u32> children;
        children.push_back(base);
        if (peekType(p) != TokenType::OP_GREATER) {
            for (;;) {
                children.push_back(parseExpr(p, 0, ParseMode::Type));
                if (peekType(p) == TokenType::CH_COMMA) { advance(p); continue; }
                break;
            }
        }
        if (!expect(p, TokenType::OP_GREATER, ErrorKind::ExpectedRightAngle, "expected '>' to close generic args")) { /* best-effort */ }
        const u32 firstChild = appendChildren(p.ast, children);
        return addNode(p.ast, ASTNode{
            .kind = NodeKind::GenericApp, .flags = 0, .tokenIndex = ltTokenIdx,
            .firstChild = firstChild, .childCount = static_cast<u32>(children.size()), .payload = 0});
    }

    //-------------------- OP_LESS infix handler --------------------
    //
    //OP_LESS is a multi-purpose token in value-mode:
    //  <<=  shift-assign (tier 3, right-assoc)
    //  <<   shift        (tier 12, left-assoc)
    //  <=   compare      (tier 8, non-chain)
    //  <    compare or generic-app — speculate first, fall back to compare
    //
    //Cursor is at the first OP_LESS.
    [[nodiscard]] inline u32 inLess(Parser& p, u32 left, int rbp, ParseMode mode) noexcept {
        const u64 saved = p.cursor;
        if (mode == ParseMode::Type) {
            //In type mode, '<' opens a generic argument list. The infix should never fire at
            //value-context rbp since type-mode parseExpr doesn't recurse into `<`-as-compare.
            if (LBP_POSTFIX <= rbp) return left;
            const u32 ltIdx = currentTokenIndex(p);
            advance(p);
            return buildGenericApp(p, left, ltIdx);
        }

        //Try `<<` / `<<=`
        const u32 firstIdx = currentTokenIndex(p);
        advance(p); //first '<'
        if (matchRaw(p, TokenType::OP_LESS)) {
            if (matchRaw(p, TokenType::OP_EQUAL)) {
                if (LBP_ASSIGN <= rbp) { p.cursor = saved; return left; }
                const u32 right = parseExpr(p, LBP_ASSIGN - 1, mode);
                return makeBinaryOp(p, left, right, firstIdx, TokenType::OP_DOUBLE_LESS_EQUAL);
            }
            if (LBP_SHIFT <= rbp) { p.cursor = saved; return left; }
            const u32 right = parseExpr(p, LBP_SHIFT, mode);
            return makeBinaryOp(p, left, right, firstIdx, TokenType::OP_DOUBLE_LESS);
        }
        if (matchRaw(p, TokenType::OP_EQUAL)) {
            //'<='
            if (LBP_COMPARE <= rbp) { p.cursor = saved; return left; }
            const u32 right = parseExpr(p, LBP_COMPARE, mode);
            const u32 node = makeBinaryOp(p, left, right, firstIdx, TokenType::OP_LESS_EQUAL);
            if (isComparison(peekType(p))) recordError(p, ErrorKind::ComparisonChained);
            return node;
        }
        //Plain '<' — speculate as generic-app first.
        p.cursor = saved;
        const u64 specSaved = p.cursor;
        if (tryParseGenericArgs(p)) {
            //success: cursor is now past the closing '>'. Restore to '<' and let the real parser walk.
            p.cursor = specSaved;
            const u32 ltIdx = currentTokenIndex(p);
            advance(p); //consume '<'
            return buildGenericApp(p, left, ltIdx);
        }
        //Comparison.
        if (LBP_COMPARE <= rbp) { p.cursor = saved; return left; }
        advance(p); //consume '<'
        const u32 right = parseExpr(p, LBP_COMPARE, mode);
        const u32 node = makeBinaryOp(p, left, right, firstIdx, TokenType::OP_LESS);
        if (isComparison(peekType(p))) recordError(p, ErrorKind::ComparisonChained);
        return node;
    }

    //OP_GREATER: comparison or shift. Generic close is handled by buildGenericApp.
    [[nodiscard]] inline u32 inGreater(Parser& p, u32 left, int rbp, ParseMode mode) noexcept {
        const u64 saved = p.cursor;
        const u32 firstIdx = currentTokenIndex(p);
        advance(p);
        if (matchRaw(p, TokenType::OP_GREATER)) {
            if (matchRaw(p, TokenType::OP_EQUAL)) {
                if (LBP_ASSIGN <= rbp) { p.cursor = saved; return left; }
                const u32 right = parseExpr(p, LBP_ASSIGN - 1, mode);
                return makeBinaryOp(p, left, right, firstIdx, TokenType::OP_DOUBLE_GREATER_EQUAL);
            }
            if (LBP_SHIFT <= rbp) { p.cursor = saved; return left; }
            const u32 right = parseExpr(p, LBP_SHIFT, mode);
            return makeBinaryOp(p, left, right, firstIdx, TokenType::OP_DOUBLE_GREATER);
        }
        if (matchRaw(p, TokenType::OP_EQUAL)) {
            if (LBP_COMPARE <= rbp) { p.cursor = saved; return left; }
            const u32 right = parseExpr(p, LBP_COMPARE, mode);
            const u32 node = makeBinaryOp(p, left, right, firstIdx, TokenType::OP_GREATER_EQUAL);
            if (isComparison(peekType(p))) recordError(p, ErrorKind::ComparisonChained);
            return node;
        }
        if (LBP_COMPARE <= rbp) { p.cursor = saved; return left; }
        const u32 right = parseExpr(p, LBP_COMPARE, mode);
        const u32 node = makeBinaryOp(p, left, right, firstIdx, TokenType::OP_GREATER);
        if (isComparison(peekType(p))) recordError(p, ErrorKind::ComparisonChained);
        return node;
    }

    //-------------------- rule table --------------------

    using PrefixFn = u32(*)(Parser&, ParseMode) noexcept;
    using InfixFn  = u32(*)(Parser&, u32 left, int rbp, ParseMode) noexcept;

    struct ParseRule {
        PrefixFn prefix;
        InfixFn  infix;
        int      lbp;
    };

    //Total token-kind count includes MISC_ERROR as the last variant.
    constexpr size_t TOKEN_KIND_COUNT = static_cast<size_t>(TokenType::MISC_ERROR) + 1;

    //Construct the value-mode rule table.
    [[nodiscard]] inline array<ParseRule, TOKEN_KIND_COUNT> buildValueRules() noexcept {
        array<ParseRule, TOKEN_KIND_COUNT> r{};
        for (auto& slot : r) slot = ParseRule{nullptr, nullptr, LBP_NONE};

        auto setPref = [&](TokenType t, PrefixFn f) noexcept { r[(size_t)t].prefix = f; };
        auto setIn   = [&](TokenType t, InfixFn f, int lbp) noexcept { r[(size_t)t].infix = f; r[(size_t)t].lbp = lbp; };

        using enum TokenType;

        //Prefix: identifiers and literals
        setPref(LT_IDENTIFIER, prefIdentifier);
        setPref(LT_STRING, prefStringLiteral);
        setPref(LT_CHAR, prefCharLiteral);
        setPref(LT_LABEL, prefLabelLiteral);
        for (TokenType t : {LT_BINARY_INT, LT_BINARY_INT_U8, LT_BINARY_INT_U16, LT_BINARY_INT_U32, LT_BINARY_INT_U64,
                            LT_BINARY_INT_I8, LT_BINARY_INT_I16, LT_BINARY_INT_I32, LT_BINARY_INT_I64,
                            LT_OCTAL_INT,  LT_OCTAL_INT_U8,  LT_OCTAL_INT_U16,  LT_OCTAL_INT_U32,  LT_OCTAL_INT_U64,
                            LT_OCTAL_INT_I8,  LT_OCTAL_INT_I16,  LT_OCTAL_INT_I32,  LT_OCTAL_INT_I64,
                            LT_DECIMAL_INT, LT_DECIMAL_INT_U8, LT_DECIMAL_INT_U16, LT_DECIMAL_INT_U32, LT_DECIMAL_INT_U64,
                            LT_DECIMAL_INT_I8, LT_DECIMAL_INT_I16, LT_DECIMAL_INT_I32, LT_DECIMAL_INT_I64,
                            LT_HEX_INT, LT_HEX_INT_U8, LT_HEX_INT_U16, LT_HEX_INT_U32, LT_HEX_INT_U64,
                            LT_HEX_INT_I8, LT_HEX_INT_I16, LT_HEX_INT_I32, LT_HEX_INT_I64})
            setPref(t, prefIntLiteral);
        for (TokenType t : {LT_DECIMAL_FLOAT, LT_DECIMAL_FLOAT_F32, LT_DECIMAL_FLOAT_F64,
                            LT_HEX_FLOAT, LT_HEX_FLOAT_F32, LT_HEX_FLOAT_F64})
            setPref(t, prefFloatLiteral);
        setPref(KW_SELF, prefSelf);
        setPref(KW_VOID, prefVoid);
        setPref(KW_AUTO, prefAuto);
        setPref(KW_DUP,  prefDup);
        setPref(CH_LEFT_PAREN, prefParen);

        //Prefix unary
        setPref(OP_BANG,        prefUnary);
        setPref(OP_DOUBLE_BANG, prefUnary); //diagnoses
        setPref(OP_TILDE,       prefUnary);
        setPref(OP_DASH,        prefUnary);
        setPref(OP_PLUS,        prefUnary);
        setPref(OP_STAR,        prefUnary);
        setPref(OP_AMPERSAND,   prefUnary);
        setPref(OP_DOUBLE_PLUS, prefUnary); //diagnoses
        setPref(OP_DOUBLE_DASH, prefUnary); //diagnoses

        //Infix arithmetic
        setIn(OP_PLUS,    inAdd, LBP_ADD);
        setIn(OP_DASH,    inAdd, LBP_ADD);
        setIn(OP_STAR,    inMul, LBP_MUL);
        setIn(OP_SLASH,   inMul, LBP_MUL);
        setIn(OP_PERCENT, inMul, LBP_MUL);

        //Logical
        setIn(OP_DOUBLE_AMPERSAND, inAnd, LBP_AND);
        setIn(OP_DOUBLE_BAR,       inOr,  LBP_OR);

        //Bitwise
        setIn(OP_AMPERSAND, inBitAnd, LBP_BIT_AND);
        setIn(OP_BAR,       inBitOr,  LBP_BIT_OR);
        setIn(OP_CARET,     inBitXor, LBP_BIT_XOR);

        //Comparison
        setIn(OP_DOUBLE_EQUAL, inEqNeq, LBP_COMPARE);
        setIn(OP_BANG_EQUAL,   inEqNeq, LBP_COMPARE);
        setIn(OP_LESS,         inLess,    LBP_OP_LESS);
        setIn(OP_GREATER,      inGreater, LBP_OP_GREATER);
        setIn(OP_DOUBLE_LESS,    inShift, LBP_SHIFT);
        setIn(OP_DOUBLE_GREATER, inShift, LBP_SHIFT);
        //The lexer doesn't currently emit OP_LESS_EQUAL etc. directly, but we register
        //them in case future lex passes fuse them.
        setIn(OP_LESS_EQUAL,    inEqNeq, LBP_COMPARE);
        setIn(OP_GREATER_EQUAL, inEqNeq, LBP_COMPARE);

        //Assignment family (all right-assoc, tier 3)
        setIn(OP_EQUAL,            inAssign, LBP_ASSIGN);
        setIn(OP_PLUS_EQUAL,       inAssign, LBP_ASSIGN);
        setIn(OP_DASH_EQUAL,       inAssign, LBP_ASSIGN);
        setIn(OP_STAR_EQUAL,       inAssign, LBP_ASSIGN);
        setIn(OP_SLASH_EQUAL,      inAssign, LBP_ASSIGN);
        setIn(OP_PERCENT_EQUAL,    inAssign, LBP_ASSIGN);
        setIn(OP_AMPERSAND_EQUAL,  inAssign, LBP_ASSIGN);
        setIn(OP_BAR_EQUAL,        inAssign, LBP_ASSIGN);
        setIn(OP_CARET_EQUAL,      inAssign, LBP_ASSIGN);
        setIn(OP_DOUBLE_LESS_EQUAL,    inAssign, LBP_ASSIGN);
        setIn(OP_DOUBLE_GREATER_EQUAL, inAssign, LBP_ASSIGN);

        //Range
        setIn(OP_DOUBLE_PERIOD, inRange, LBP_RANGE);
        setIn(OP_TRIPLE_PERIOD, inRange, LBP_RANGE);

        //Ternary
        setIn(OP_QUESTION_COLON, inTernaryQ,  LBP_TERNARY);

        //`as` cast
        setIn(KW_AS, inAs, LBP_AS);

        //Postfix (tier 17)
        setIn(OP_BANG,        inPostfixUnary, LBP_POSTFIX);
        setIn(OP_DOUBLE_BANG, inPostfixUnary, LBP_POSTFIX);
        setIn(OP_DOUBLE_PLUS, inPostfixUnary, LBP_POSTFIX);
        setIn(OP_DOUBLE_DASH, inPostfixUnary, LBP_POSTFIX);

        //Member access
        setIn(OP_PERIOD,       inMemberAccess, LBP_POSTFIX);
        setIn(OP_DASH_GREATER, inMemberAccess, LBP_POSTFIX);
        setIn(OP_COLON,        inMemberAccess, LBP_POSTFIX);

        //Call / index / struct literal
        setIn(CH_LEFT_PAREN,   inCall,      LBP_POSTFIX);
        setIn(CH_LEFT_BRACKET, inIndex,     LBP_POSTFIX);
        setIn(CH_LEFT_BRACE,   inStructLit, LBP_POSTFIX);

        return r;
    }

    //Type-mode rule table: prefix `~`, postfix `!` `!!` `*` `&`, generic-app at `<`.
    //No comparison, no arithmetic.
    [[nodiscard]] inline array<ParseRule, TOKEN_KIND_COUNT> buildTypeRules() noexcept {
        array<ParseRule, TOKEN_KIND_COUNT> r{};
        for (auto& slot : r) slot = ParseRule{nullptr, nullptr, LBP_NONE};

        auto setPref = [&](TokenType t, PrefixFn f) noexcept { r[(size_t)t].prefix = f; };
        auto setIn   = [&](TokenType t, InfixFn f, int lbp) noexcept { r[(size_t)t].infix = f; r[(size_t)t].lbp = lbp; };

        using enum TokenType;
        setPref(LT_IDENTIFIER, prefIdentifier);
        setPref(KW_VOID,       prefVoid);
        setPref(KW_AUTO,       prefAuto);
        setPref(OP_DOUBLE_QUESTION, prefNoReturn);
        setPref(OP_TILDE,      prefUnary);

        setIn(OP_LESS,         inLess,         LBP_POSTFIX);
        setIn(OP_BANG,         inPostfixUnary, LBP_POSTFIX);
        setIn(OP_DOUBLE_BANG,  inPostfixUnary, LBP_POSTFIX);
        setIn(OP_STAR,         inPostfixUnary, LBP_POSTFIX);
        setIn(OP_AMPERSAND,    inPostfixUnary, LBP_POSTFIX);
        setIn(OP_COLON,        inMemberAccess, LBP_POSTFIX);
        setIn(OP_DOUBLE_COLON, inMemberAccess, LBP_POSTFIX);

        return r;
    }

    inline const array<ParseRule, TOKEN_KIND_COUNT> valueRules = buildValueRules();
    inline const array<ParseRule, TOKEN_KIND_COUNT> typeRules  = buildTypeRules();

    [[nodiscard]] inline const ParseRule& ruleFor(TokenType t, ParseMode mode) noexcept {
        return mode == ParseMode::Type ? typeRules[(size_t)t] : valueRules[(size_t)t];
    }

    //-------------------- main Pratt loop --------------------

    [[nodiscard]] inline u32 parseExpr(Parser& p, int rbp, ParseMode mode) noexcept {
        skipTrivia(p);
        const TokenType firstType = peekType(p);
        const ParseRule& prefRule = ruleFor(firstType, mode);
        if (!prefRule.prefix) {
            recordError(p, ErrorKind::ExpectedExpression);
            return makeErrorNode(p);
        }
        u32 left = prefRule.prefix(p, mode);
        for (;;) {
            skipTrivia(p);
            const TokenType t = peekType(p);
            if (p.stopAtConditionBrace && t == TokenType::CH_LEFT_BRACE && !braceOpensConditionStructLit(p, left)) break;
            const ParseRule& inRule = ruleFor(t, mode);
            if (!inRule.infix || inRule.lbp <= rbp) break;
            const u64 before = p.cursor;
            left = inRule.infix(p, left, rbp, mode);
            if (p.cursor == before) break; // handler refused; e.g., < as compare against high rbp
        }
        return left;
    }
}