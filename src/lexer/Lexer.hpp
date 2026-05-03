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

        if (cursor == fileContent.size()) return {.type = MISC_EOF};
        else if (cursor > fileContent.size()) {
            cerr << "How did we get here?\n";
            return {.type = MISC_EOF};
        }
        if (isWhitespace(fileContent[cursor])) {
            const u64 start = cursor;
            while (cursor < fileContent.size() && isWhitespace(fileContent[cursor])) cursor++;
            //note: We don't include the whitespace payload in the token to prettify the lexer output. And we probably don't need it for anything else either, so this is fine.
            return {.type = MISC_WHITESPACE};
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
                        return {.type = OP_DOUBLE_PLUS};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_PLUS_EQUAL};
                    }
                }
                cursor++;
                return {.type = OP_PLUS};
            case '-':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '-') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_DASH};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_DASH_EQUAL};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return {.type = OP_DASH_GREATER};
                    }
                }
                cursor++;
                return {.type = OP_DASH};
            case '*':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_STAR_EQUAL};
                    }
                }
                cursor++;
                return {.type = OP_STAR};
            case '/':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_SLASH_EQUAL};
                    }
                    else if (fileContent[cursor + 1] == '/') {
                        //Line comment, skip until end of line or end of file
                        cursor += 2;
                        while (cursor < fileContent.size() && fileContent[cursor] != '\n') cursor++;
                        if (cursor == fileContent.size()) return {.type = MISC_EOF};
                        else return getNextToken();
                    }
                    else if (fileContent[cursor + 1] == '*') {
                        //Multiline comment, skip until closing */
                        cursor += 2;
                        const u64 origin = cursor - 2;
                        while (cursor + 1 < fileContent.size() && !(fileContent[cursor] == '*' && fileContent[cursor + 1] == '/')) cursor++;
                        if (cursor + 1 == fileContent.size()) {
                            //Unterminated multiline comment
                            //note: The file has ended, but comments don't have a token type and we need to return something, so we're forced to just return EOF.
                            cursor++;
                            return {.type = MISC_EOF};
                        }
                        else {
                            cursor += 2;
                            return getNextToken();
                        }
                    }
                }
                cursor++;
                return {.type = OP_SLASH};
            case '%':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return {.type = OP_PERCENT_EQUAL};
                }
                cursor++;
                return {.type = OP_PERCENT};
            case '<':
                //Don't eat as multicharacter operators yet, because it might be in `op_<<<type T>` or something.
                cursor++;
                return {.type = OP_LESS};
            case '>':
                //Don't eat as multicharacter operators yet, because it might be a generics closer. The classic vector<vector<int`>>` bug and all that.
                cursor++;
                return {.type = OP_GREATER};
            case '|':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '|') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_BAR};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_BAR_EQUAL};
                    }
                }
                cursor++;
                return {.type = OP_BAR};
            case '&':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '&') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_AMPERSAND};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_AMPERSAND_EQUAL};
                    }
                }
                cursor++;
                return {.type = OP_AMPERSAND};
            case '^':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '=') {
                    cursor += 2;
                    return {.type = OP_CARET_EQUAL};
                }
                cursor++;
                return {.type = OP_CARET};
            case '~':
                cursor++;
                return {.type = OP_TILDE};
            case '!':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '!') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_BANG};
                    }
                    else if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_BANG_EQUAL};
                    }
                }
                cursor++;
                return {.type = OP_BANG};
            case '=':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '=') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_EQUAL};
                    }
                    else if (fileContent[cursor + 1] == '>') {
                        cursor += 2;
                        return {.type = OP_EQUAL_GREATER};
                    }
                }
                cursor++;
                return {.type = OP_EQUAL};
            case '?':
                if (cursor + 1 < fileContent.size()) {
                    if (fileContent[cursor + 1] == '?') {
                        cursor += 2;
                        return {.type = OP_DOUBLE_QUESTION};
                    }
                    else if (fileContent[cursor + 1] == ':') {
                        cursor += 2;
                        return {.type = OP_QUESTION_COLON};
                    }
                }
                cursor++;
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 1, 1)};
            case ':':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == ':') {
                    cursor += 2;
                    return {.type = OP_DOUBLE_COLON};
                }
                cursor++;
                return {.type = OP_COLON};
            case '.':
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '.') {
                    if (cursor + 2 < fileContent.size() && fileContent[cursor + 2] == '.') {
                        cursor += 3;
                        return {.type = OP_TRIPLE_PERIOD};
                    }
                    else {
                        cursor += 2;
                        return {.type = OP_DOUBLE_PERIOD};
                    }
                }
                cursor++;
                return {.type = OP_PERIOD};
            case ';':
                cursor++;
                return {.type = CH_SEMICOLON};
            case ',':
                cursor++;
                return {.type = CH_COMMA};
            case '\'': return getCharLiteralOrLabel(cursor);
            case '(':
                cursor++;
                return {.type = CH_LEFT_PAREN};
            case ')':
                cursor++;
                return {.type = CH_RIGHT_PAREN};
            case '{':
                cursor++;
                return {.type = CH_LEFT_BRACE};
            case '}':
                cursor++;
                return {.type = CH_RIGHT_BRACE};
            case '[':
                cursor++;
                return {.type = CH_LEFT_BRACKET};
            case ']':
                cursor++;
                return {.type = CH_RIGHT_BRACKET};
            case '"': return getStringLiteral(cursor);
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                return getNumberLiteral(cursor);
            default: return getIdentifierLike(cursor);
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