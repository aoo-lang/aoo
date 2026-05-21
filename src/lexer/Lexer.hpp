#pragma once
#include <iostream>
#include <span>
#include <string>

#include "../currentFile.hpp"
#include "charLiteral.hpp"
#include "identifier.hpp"
#include "numberLiteral.hpp"
#include "stringLiteral.hpp"
#include "tokens.hpp"
#include "utils.hpp"
#include "variables.hpp"

namespace AOO::Lexer {
    namespace detail {

    typedef uint64_t u64;
    using std::cerr, std::string, std::span;
    using namespace AOO::Lexer::internal;

    [[nodiscard]] inline Token scanRealToken() noexcept {
        using enum TokenType;

        if (cursor == fileContent.size()) return makeToken(MISC_EOF, cursor, cursor);
        else if (cursor > fileContent.size()) {
            cerr << "How did we get here?\n";
            return makeToken(MISC_EOF, fileContent.size(), fileContent.size());
        }
        const u64 origin = cursor;
        switch (fileContent[cursor]) {
            //BOM mark `EF BB BF`
            case 239:
                cursor++;
                return makeToken(MISC_ERROR, origin, cursor);
            case '+':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '+') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_PLUS, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_PLUS_EQUAL, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_PLUS, origin, cursor);
            case '-':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '-') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_DASH, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_DASH_EQUAL, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return makeToken(OP_DASH_GREATER, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_DASH, origin, cursor);
            case '*':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_STAR_EQUAL, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_STAR, origin, cursor);
            case '/':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_SLASH_EQUAL, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_SLASH, origin, cursor);
            case '%':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return makeToken(OP_PERCENT_EQUAL, origin, cursor);
                }
                cursor++;
                return makeToken(OP_PERCENT, origin, cursor);
            case '<':
                //Don't eat as multicharacter operators yet, because it might be in `op_<<<type T>` or something.
                cursor++;
                return makeToken(OP_LESS, origin, cursor);
            case '>':
                //Don't eat as multicharacter operators yet, because it might be a generics closer. The classic vector<vector<int`>>` bug and all that.
                cursor++;
                return makeToken(OP_GREATER, origin, cursor);
            case '|':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '|') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_BAR, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_BAR_EQUAL, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_BAR, origin, cursor);
            case '&':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '&') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_AMPERSAND, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_AMPERSAND_EQUAL, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_AMPERSAND, origin, cursor);
            case '^':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return makeToken(OP_CARET_EQUAL, origin, cursor);
                }
                cursor++;
                return makeToken(OP_CARET, origin, cursor);
            case '~':
                cursor++;
                return makeToken(OP_TILDE, origin, cursor);
            case '!':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '!') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_BANG, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_BANG_EQUAL, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_BANG, origin, cursor);
            case '=':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_EQUAL, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return makeToken(OP_EQUAL_GREATER, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_EQUAL, origin, cursor);
            case '?':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '?') {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_QUESTION, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == ':') {
                        cursor += 2;
                        return makeToken(OP_QUESTION_COLON, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(MISC_ERROR, origin, cursor);
            case ':':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == ':') {
                    cursor += 2;
                    return makeToken(OP_DOUBLE_COLON, origin, cursor);
                }
                cursor++;
                return makeToken(OP_COLON, origin, cursor);
            case '.':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '.') {
                    if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '.') {
                        cursor += 3;
                        return makeToken(OP_TRIPLE_PERIOD, origin, cursor);
                    }
                    else {
                        cursor += 2;
                        return makeToken(OP_DOUBLE_PERIOD, origin, cursor);
                    }
                }
                cursor++;
                return makeToken(OP_PERIOD, origin, cursor);
            case ';':
                cursor++;
                return makeToken(CH_SEMICOLON, origin, cursor);
            case ',':
                cursor++;
                return makeToken(CH_COMMA, origin, cursor);
            case '\'': return getCharLiteralOrLabel(cursor);
            case '(':
                cursor++;
                return makeToken(CH_LEFT_PAREN, origin, cursor);
            case ')':
                cursor++;
                return makeToken(CH_RIGHT_PAREN, origin, cursor);
            case '{':
                cursor++;
                return makeToken(CH_LEFT_BRACE, origin, cursor);
            case '}':
                cursor++;
                return makeToken(CH_RIGHT_BRACE, origin, cursor);
            case '[':
                cursor++;
                return makeToken(CH_LEFT_BRACKET, origin, cursor);
            case ']':
                cursor++;
                return makeToken(CH_RIGHT_BRACKET, origin, cursor);
            case '"': return getStringLiteral(cursor);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                return getNumberLiteral(cursor);
            default: return getIdentifierLike(cursor);
        }
    }

    [[nodiscard]] inline Token getNextToken() noexcept {
        const span<const u8> leadingTrivia = internal::scanLeadingTrivia(internal::cursor);
        Token token = scanRealToken();
        token.leadingTrivia = leadingTrivia;
        token.trailingTrivia = internal::scanTrailingTrivia(internal::cursor);
        return token;
    }

    } // namespace detail

    inline void reset() noexcept {
        internal::cursor = 0;
        internal::tokens.clear();
    }

    inline void parseTokens() noexcept {
        Token token{};
        do {
            token = detail::getNextToken();
            internal::tokens.push_back(token);
        } while (token.type != TokenType::MISC_EOF);
    }

    inline const vector<Token>& getTokenStorage() noexcept { return internal::tokens; }
}