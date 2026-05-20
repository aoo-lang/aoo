#pragma once
#include <iostream>
#include <span>
#include <string>

#include "../currentFile.hpp"
#include "../util/string.hpp"
#include "charLiteral.hpp"
#include "identifier.hpp"
#include "numberLiteral.hpp"
#include "stringLiteral.hpp"
#include "tokens.hpp"
#include "utils.hpp"

namespace AOO::Lexer {
    typedef uint64_t u64;
    using std::cerr, std::string, std::span, Util::isWhitespace;

    namespace detail {
        inline u64 cursor{0};

        [[nodiscard]] inline bool startsWith(u8 first, u8 second) noexcept {
            return cursor + 1 < fileContent.size() && fileContent[cursor] == first && fileContent[cursor + 1] == second;
        }

        [[nodiscard]] inline bool startsWithBom() noexcept {
            return cursor == 0
                && fileContent.size() >= 3
                && fileContent[0] == 239
                && fileContent[1] == 187
                && fileContent[2] == 191;
        }

        [[nodiscard]] inline bool isHorizontalWhitespace(u8 c) noexcept {
            return c == ' ' || c == '\t';
        }

        [[nodiscard]] inline bool startsNewline() noexcept {
            return cursor < fileContent.size() && (fileContent[cursor] == '\n' || fileContent[cursor] == '\r');
        }

        inline void consumeNewline() noexcept {
            if (cursor < fileContent.size() && fileContent[cursor] == '\r' && cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '\n') {
                cursor += 2;
            }
            else if (cursor < fileContent.size()) cursor++;
        }

        inline void consumeLineComment() noexcept {
            cursor += 2;
            while (cursor < fileContent.size() && fileContent[cursor] != '\n' && fileContent[cursor] != '\r') cursor++;
        }

        [[nodiscard]] inline bool consumeBlockComment() noexcept {
            bool containsNewline = false;
            cursor += 2;
            while (cursor + 1 < fileContent.size() && !(fileContent[cursor] == '*' && fileContent[cursor + 1] == '/')) {
                if (fileContent[cursor] == '\r') {
                    containsNewline = true;
                    if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '\n') cursor += 2;
                    else cursor++;
                }
                else {
                    if (fileContent[cursor] == '\n') containsNewline = true;
                    cursor++;
                }
            }
            if (cursor + 1 < fileContent.size()) cursor += 2;
            else cursor = fileContent.size();
            return containsNewline;
        }

        [[nodiscard]] inline bool startsTrivia() noexcept {
            return cursor < fileContent.size()
                && (isWhitespace(fileContent[cursor]) || startsWith('/', '/') || startsWith('/', '*') || startsWithBom());
        }

        [[nodiscard]] inline span<const u8> scanLeadingTrivia() noexcept {
            const u64 origin = cursor;
            while (startsTrivia()) {
                if (startsWithBom()) cursor += 3;
                else if (isWhitespace(fileContent[cursor])) cursor++;
                else if (startsWith('/', '/')) consumeLineComment();
                else if (startsWith('/', '*')) (void)consumeBlockComment();
            }
            return makePayload(origin, cursor);
        }

        [[nodiscard]] inline span<const u8> scanTrailingTrivia() noexcept {
            const u64 origin = cursor;
            while (cursor < fileContent.size()) {
                if (isHorizontalWhitespace(fileContent[cursor])) {
                    cursor++;
                }
                else if (startsWith('/', '/')) {
                    consumeLineComment();
                }
                else if (startsNewline()) {
                    consumeNewline();
                    break;
                }
                else if (startsWith('/', '*')) {
                    const bool containsNewline = consumeBlockComment();
                    if (containsNewline) break;
                }
                else break;
            }
            return makePayload(origin, cursor);
        }
    }

    inline vector<Token> tokens;

    inline void init() noexcept {
        detail::cursor = 0;
        tokens.clear();
    }

    [[nodiscard]] inline Token scanRealToken() noexcept {
        using namespace detail;
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
        const span<const u8> leadingTrivia = detail::scanLeadingTrivia();
        Token token = scanRealToken();
        token.leadingTrivia = leadingTrivia;
        token.trailingTrivia = detail::scanTrailingTrivia();
        return token;
    }

    inline void parse() noexcept {
        Token token{};
        do {
            token = getNextToken();
            tokens.push_back(token);
        } while (token.type != TokenType::MISC_EOF);
    }
}