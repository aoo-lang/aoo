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

namespace AO::Lexer {
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
        using enum StringType;
        using AO::fileContent;
        if (cursor == fileContent.size()) return {.type = MISC_EOF, .strType = NotAString, .payload = {}};
        else if (cursor > fileContent.size()) {
            cerr << "How did we get here?\n";
            return {.type = MISC_EOF, .strType = NotAString, .payload = {}};
        }
        if (isWhitespace(fileContent[cursor])) {
            const u64 start = cursor;
            while (cursor < fileContent.size() && isWhitespace(fileContent[cursor])) cursor++;
            //note: We don't include the whitespace payload in the token to prettify the lexer output. And we probably don't need it for anything else either, so this is fine.
            return {.type = MISC_WHITESPACE, .strType = NotAString, .payload = {}};
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
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 1, 1)};
                }
            case '+':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '+') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_PLUS, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_PLUS_EQUAL, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_PLUS, .strType = NotAString, .payload = {}};
            case '-':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '-') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_DASH, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_DASH_EQUAL, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return {.type = OP_DASH_GREATER, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_DASH, .strType = NotAString, .payload = {}};
            case '*':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_STAR_EQUAL, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '*') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_STAR, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_STAR, .strType = NotAString, .payload = {}};
            case '/':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_SLASH_EQUAL, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '/') {
                        //Line comment, skip until end of line or end of file
                        cursor += 2;
                        while (cursor < fileContent.size() && fileContent[cursor] != '\n') cursor++;
                        if (cursor == fileContent.size()) return {.type = MISC_EOF, .strType = NotAString, .payload = {}};
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
                            return {.type = MISC_ERROR, .strType = NotAString, .payload = {}};
                        }
                        else {
                            cursor += 2;
                            return getNextToken();
                        }
                    }
                }
                cursor++;
                return {.type = OP_SLASH, .strType = NotAString, .payload = {}};
            case '%':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return {.type = OP_PERCENT_EQUAL, .strType = NotAString, .payload = {}};
                }
                cursor++;
                return {.type = OP_PERCENT, .strType = NotAString, .payload = {}};
            case '<':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_LESS_EQUAL, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '<') {
                        if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '=') {
                            cursor += 3;
                            return {.type = OP_DOUBLE_LESS_EQUAL, .strType = NotAString, .payload = {}};
                        }
                        else {
                            cursor += 2;
                            return {.type = OP_DOUBLE_LESS, .strType = NotAString, .payload = {}};
                        }
                    }
                }
                cursor++;
                return {.type = OP_LESS, .strType = NotAString, .payload = {}};
            case '>':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_GREATER_EQUAL, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '=') {
                            cursor += 3;
                            return {.type = OP_DOUBLE_GREATER_EQUAL, .strType = NotAString, .payload = {}};
                        }
                        else {
                            cursor += 2;
                            return {.type = OP_DOUBLE_GREATER, .strType = NotAString, .payload = {}};
                        }
                    }
                }
                cursor++;
                return {.type = OP_GREATER, .strType = NotAString, .payload = {}};
            case '|':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '|') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_BAR, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_BAR_EQUAL, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_BAR, .strType = NotAString, .payload = {}};
            case '&':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '&') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_AMPERSAND, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_AMPERSAND_EQUAL, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_AMPERSAND, .strType = NotAString, .payload = {}};
            case '^':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return {.type = OP_CARET_EQUAL, .strType = NotAString, .payload = {}};
                }
                cursor++;
                return {.type = OP_CARET, .strType = NotAString, .payload = {}};
            case '!':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '!') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_BANG, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_BANG_EQUAL, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_BANG, .strType = NotAString, .payload = {}};
            case '=':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_EQUAL, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return {.type = OP_EQUAL_GREATER, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_EQUAL, .strType = NotAString, .payload = {}};
            case '?':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '?') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_QUESTION, .strType = NotAString, .payload = {}};
                    }
                    else if (fileContent[cursor + 1] == ':') {
                        cursor += 2;
                        return {.type = OP_QUESTION_COLON, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = OP_QUESTION, .strType = NotAString, .payload = {}};
            case ':':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == ':') {
                    cursor += 2;
                    return {.type = OP_DOUBLE_COLON, .strType = NotAString, .payload = {}};
                }
                cursor++;
                return {.type = CH_COLON, .strType = NotAString, .payload = {}};
            case '.':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '.') {
                    if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '.') {
                        cursor += 3;
                        return {.type = OP_TRIPLE_PERIOD, .strType = NotAString, .payload = {}};
                    }
                    else {
                        cursor += 2;
                        return {.type = OP_DOUBLE_PERIOD, .strType = NotAString, .payload = {}};
                    }
                }
                cursor++;
                return {.type = CH_PERIOD, .strType = NotAString, .payload = {}};
            case ';':
                cursor++;
                return {.type = CH_SEMICOLON, .strType = NotAString, .payload = {}};
            // We can't just take underscores now because they can be valid identifiers, so we handle them in getIdentifier() instead.
            //case '_':
            //    cursor++;
            //    return {.type = CH_UNDERSCORE, .strType = NotAString, .payload = {}};
            case ',':
                cursor++;
                return {.type = CH_COMMA, .strType = NotAString, .payload = {}};
            case '\'': return getCharLiteral(cursor);
            case '(':
                cursor++;
                return {.type = CH_LEFT_PAREN, .strType = NotAString, .payload = {}};
            case ')':
                cursor++;
                return {.type = CH_RIGHT_PAREN, .strType = NotAString, .payload = {}};
            case '{':
                cursor++;
                return {.type = CH_LEFT_BRACE, .strType = NotAString, .payload = {}};
            case '}':
                cursor++;
                return {.type = CH_RIGHT_BRACE, .strType = NotAString, .payload = {}};
            case '[':
                cursor++;
                return {.type = CH_LEFT_BRACKET, .strType = NotAString, .payload = {}};
            case ']':
                cursor++;
                return {.type = CH_RIGHT_BRACKET, .strType = NotAString, .payload = {}};
            case '"': return getStringLiteral(cursor);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': return getNumberLiteral(cursor);
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