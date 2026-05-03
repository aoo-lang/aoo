#pragma once
#include <cstdint>
#include <string_view>
#include <vector>

#include "../lexer/tokens.hpp"
#include "ASTNodes.hpp"
#include "Errors.hpp"

namespace AOO::Parser {
    typedef uint64_t u64;
    using std::vector, std::string_view, Lexer::Token, Lexer::TokenType;

    enum struct ParseMode : u8 { Value, Type };

    struct Parser {
        const vector<Token>& tokens;
        AST&                 ast;
        ErrorList&           errors;
        u64                  cursor{0};
        bool                 stopAtConditionBrace{false};
    };

    [[nodiscard]] inline bool isTrivia(TokenType t) noexcept {
        return t == TokenType::MISC_WHITESPACE;
    }

    // Advance past whitespace tokens at the cursor.
    inline void skipTrivia(Parser& p) noexcept {
        while (p.cursor < p.tokens.size() && isTrivia(p.tokens[p.cursor].type)) p.cursor++;
    }

    // Peek the current non-trivia token. Returns a synthetic EOF token if past end.
    [[nodiscard]] inline const Token& peek(Parser& p) noexcept {
        skipTrivia(p);
        if (p.cursor >= p.tokens.size()) {
            static constexpr Token eofToken{.type = TokenType::MISC_EOF};
            return eofToken;
        }
        return p.tokens[p.cursor];
    }

    // Peek the n-th non-trivia token from the current cursor (0 = current).
    [[nodiscard]] inline const Token& peekAhead(Parser& p, u64 n) noexcept {
        u64 i = p.cursor;
        while (i < p.tokens.size() && isTrivia(p.tokens[i].type)) i++;
        for (u64 step = 0; step < n; step++) {
            if (i < p.tokens.size()) i++;
            while (i < p.tokens.size() && isTrivia(p.tokens[i].type)) i++;
        }
        if (i >= p.tokens.size()) {
            static constexpr Token eofToken{.type = TokenType::MISC_EOF};
            return eofToken;
        }
        return p.tokens[i];
    }

    [[nodiscard]] inline TokenType peekType(Parser& p) noexcept { return peek(p).type; }
    [[nodiscard]] inline TokenType peekTypeAhead(Parser& p, u64 n) noexcept { return peekAhead(p, n).type; }

    [[nodiscard]] inline u32 currentTokenIndex(Parser& p) noexcept {
        skipTrivia(p);
        return static_cast<u32>(p.cursor);
    }

    [[nodiscard]] inline bool atEnd(Parser& p) noexcept {
        return peekType(p) == TokenType::MISC_EOF;
    }

    inline void advance(Parser& p) noexcept {
        skipTrivia(p);
        if (p.cursor < p.tokens.size()) p.cursor++;
    }

    [[nodiscard]] inline bool match(Parser& p, TokenType t) noexcept {
        if (peekType(p) == t) { advance(p); return true; }
        return false;
    }

    [[nodiscard]] inline bool check(Parser& p, TokenType t) noexcept {
        return peekType(p) == t;
    }

    inline void recordError(Parser& p, ErrorKind kind, string_view detail = {}) noexcept {
        p.errors.push_back({.kind = kind, .tokenIndex = currentTokenIndex(p), .detail = detail});
    }

    // Consume an expected token; record an error and do not advance if it does not match.
    inline bool expect(Parser& p, TokenType t, ErrorKind k, string_view detail = {}) noexcept {
        if (peekType(p) == t) { advance(p); return true; }
        recordError(p, k, detail);
        return false;
    }

    // Build an error node at the current position and consume one token to make progress.
    [[nodiscard]] inline u32 makeErrorNode(Parser& p) noexcept {
        const u32 tokIdx = currentTokenIndex(p);
        if (!atEnd(p)) advance(p);
        return addNode(p.ast, ASTNode{.kind = NodeKind::Error, .flags = FLAG_HAS_ERROR, .tokenIndex = tokIdx, .firstChild = 0, .childCount = 0, .payload = 0});
    }

    // Resync to a delimiter that is reasonable for statement/decl recovery.
    inline void resyncToStatementBoundary(Parser& p) noexcept {
        u32 depthParen = 0, depthBrace = 0, depthBracket = 0;
        while (!atEnd(p)) {
            const TokenType t = peekType(p);
            if (depthParen == 0 && depthBrace == 0 && depthBracket == 0) {
                if (t == TokenType::CH_SEMICOLON) { advance(p); return; }
                if (t == TokenType::CH_RIGHT_BRACE) return;
            }
            switch (t) {
                case TokenType::CH_LEFT_PAREN:    depthParen++;   break;
                case TokenType::CH_RIGHT_PAREN:   if (depthParen)   depthParen--;   break;
                case TokenType::CH_LEFT_BRACE:    depthBrace++;   break;
                case TokenType::CH_RIGHT_BRACE:   if (depthBrace)   depthBrace--;   break;
                case TokenType::CH_LEFT_BRACKET:  depthBracket++; break;
                case TokenType::CH_RIGHT_BRACKET: if (depthBracket) depthBracket--; break;
                default: break;
            }
            advance(p);
        }
    }
}