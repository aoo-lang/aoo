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

namespace AOO::Lexer {
    typedef uint64_t u64;
    using std::cerr, std::string, std::span, Util::isWhitespace;

    namespace detail {
        inline u64 cursor{0};
    }

    inline vector<Token> tokens;

    inline void init() noexcept {
        detail::cursor = 0;
        tokens.clear();
    }

    [[nodiscard]] inline Token getNextToken() noexcept {
        using namespace detail;
        using enum TokenType;
        using AOO::fileContent;

        if (cursor == fileContent.size()) return {.type = MISC_EOF, .payload = {}};
        else if (cursor > fileContent.size()) {
            cerr << "How did we get here?\n";
            return {.type = MISC_EOF, .payload = {}};
        }
        if (isWhitespace(fileContent[cursor])) {
            const u64 start = cursor;
            while (cursor < fileContent.size() && isWhitespace(fileContent[cursor])) cursor++;
            //note: We don't include the whitespace payload in the token to prettify the lexer output. And we probably don't need it for anything else either, so this is fine.
            return {.type = MISC_WHITESPACE, .payload = {}};
        }
        switch (fileContent[cursor]) {
            //BOM mark `EF BB BF`
            case 239:
                if (cursor + 2 < fileContent.size() && fileContent[cursor + 1] == 187 && fileContent[cursor + 2] == 191) {
                    cursor += 3;
                    return getNextToken();
                }
                else {
                    cursor++;
                    return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 1, 1)};
                }
            case '+':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '+') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_PLUS, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_PLUS_EQUAL, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_PLUS, .payload = {}};
            case '-':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '-') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_DASH, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_DASH_EQUAL, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return {.type = OP_DASH_GREATER, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_DASH, .payload = {}};
            case '*':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_STAR_EQUAL, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '*') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_STAR, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_STAR, .payload = {}};
            case '/':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_SLASH_EQUAL, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '/') {
                        //Line comment, skip until end of line or end of file
                        cursor += 2;
                        while (cursor < fileContent.size() && fileContent[cursor] != '\n') cursor++;
                        if (cursor == fileContent.size()) return {.type = MISC_EOF, .payload = {}};
                        else return getNextToken();
                    }
                    else if (fileContent[cursor + 1] == '*') {
                        //Multiline comment, skip until closing */
                        cursor += 2;
                        while (cursor + 1 < fileContent.size() && !(fileContent[cursor] == '*' && fileContent[cursor + 1] == '/')) cursor++;
                        if (cursor + 1 == fileContent.size()) {
                            //Unterminated multiline comment
                            //note: We advance the cursor to the end of the file to prevent reading the last character as a valid token in the next call.
                            cursor++;
                            return {.type = MISC_ERROR, .payload = {}};
                        }
                        else {
                            cursor += 2;
                            return getNextToken();
                        }
                    }
                }
                cursor++;
                return {.type = OP_SLASH, .payload = {}};
            case '%':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return {.type = OP_PERCENT_EQUAL, .payload = {}};
                }
                cursor++;
                return {.type = OP_PERCENT, .payload = {}};
            case '<':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_LESS_EQUAL, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '<') {
                        if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '=') {
                            cursor += 3;
                            return {.type = OP_DOUBLE_LESS_EQUAL, .payload = {}};
                        }
                        else {
                            cursor += 2;
                            return {.type = OP_DOUBLE_LESS, .payload = {}};
                        }
                    }
                }
                cursor++;
                return {.type = OP_LESS, .payload = {}};
            case '>':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_GREATER_EQUAL, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '=') {
                            cursor += 3;
                            return {.type = OP_DOUBLE_GREATER_EQUAL, .payload = {}};
                        }
                        else {
                            cursor += 2;
                            return {.type = OP_DOUBLE_GREATER, .payload = {}};
                        }
                    }
                }
                cursor++;
                return {.type = OP_GREATER, .payload = {}};
            case '|':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '|') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_BAR, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_BAR_EQUAL, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_BAR, .payload = {}};
            case '&':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '&') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_AMPERSAND, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_AMPERSAND_EQUAL, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_AMPERSAND, .payload = {}};
            case '^':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return {.type = OP_CARET_EQUAL, .payload = {}};
                }
                cursor++;
                return {.type = OP_CARET, .payload = {}};
            case '~':
                cursor++;
                return {.type = OP_TILDE, .payload = {}};
            case '!':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '!') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_BANG, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_BANG_EQUAL, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_BANG, .payload = {}};
            case '=':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_EQUAL, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return {.type = OP_EQUAL_GREATER, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_EQUAL, .payload = {}};
            case '?':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '?') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_QUESTION, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == ':') {
                        cursor += 2;
                        return {.type = OP_QUESTION_COLON, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_QUESTION, .payload = {}};
            case ':':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == ':') {
                    cursor += 2;
                    return {.type = OP_DOUBLE_COLON, .payload = {}};
                }
                cursor++;
                return {.type = CH_COLON, .payload = {}};
            case '.':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '.') {
                    if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '.') {
                        cursor += 3;
                        return {.type = OP_TRIPLE_PERIOD, .payload = {}};
                    }
                    else {
                        cursor += 2;
                        return {.type = OP_DOUBLE_PERIOD, .payload = {}};
                    }
                }
                cursor++;
                return {.type = CH_PERIOD, .payload = {}};
            case ';':
                cursor++;
                return {.type = CH_SEMICOLON, .payload = {}};
            // We can't just take underscores now because they can be valid identifiers, so we handle them in getIdentifier() instead.
            //case '_':
            //    cursor++;
            //    return {.type = CH_UNDERSCORE, .payload = {}};
            case ',':
                cursor++;
                return {.type = CH_COMMA, .payload = {}};
            case '\'': return getCharLiteral(cursor);
            case '(':
                cursor++;
                return {.type = CH_LEFT_PAREN, .payload = {}};
            case ')':
                cursor++;
                return {.type = CH_RIGHT_PAREN, .payload = {}};
            case '{':
                cursor++;
                return {.type = CH_LEFT_BRACE, .payload = {}};
            case '}':
                cursor++;
                return {.type = CH_RIGHT_BRACE, .payload = {}};
            case '[':
                cursor++;
                return {.type = CH_LEFT_BRACKET, .payload = {}};
            case ']':
                cursor++;
                return {.type = CH_RIGHT_BRACKET, .payload = {}};
            case '"': return getStringLiteral(cursor);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': return getNumberLiteral(cursor);
            case 'b': case 'c': case 'r': case 'f':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '"') return getStringLiteral(cursor);
                else [[fallthrough]];
            default: return getIdentifier(cursor);
        }
    }

    inline void parse() noexcept {
        Token token{};
        do {
            token = getNextToken();
            tokens.push_back(token);
        } while (token.type != TokenType::MISC_EOF);
    }
}