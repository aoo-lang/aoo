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
    }

    inline vector<Token> tokens;

    inline void init() noexcept {
        detail::cursor = 0;
        tokens.clear();
    }

    [[nodiscard]] inline Token getNextToken() noexcept {
        using namespace detail;
        using enum TokenType;

        if (cursor == fileContent.size()) return makeToken(MISC_EOF, cursor, cursor);
        else if (cursor > fileContent.size()) {
            cerr << "How did we get here?\n";
            return makeToken(MISC_EOF, fileContent.size(), fileContent.size());
        }
        if (isWhitespace(fileContent[cursor])) {
            const u64 start = cursor;
            while (cursor < fileContent.size() && isWhitespace(fileContent[cursor])) cursor++;
            return makeToken(MISC_WHITESPACE, start, cursor);
        }
        const u64 origin = cursor;
        switch (fileContent[cursor]) {
            //BOM mark `EF BB BF`
            case 239:
                if (cursor == 0 && fileContent.size() >= 3 && fileContent[1] == 187 && fileContent[2] == 191) {
                    cursor += 3;
                    return getNextToken();
                }
                else {
                    cursor++;
                    return makeToken(MISC_ERROR, origin, cursor);
                }
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
                    else if (fileContent[cursor + 1] == '/') {
                        //Line comment: emit MISC_LINE_COMMENT trivia token (preserved for CST/IDE).
                        //Payload includes the leading "//" and excludes the trailing newline.
                        cursor += 2;
                        while (cursor < fileContent.size() && fileContent[cursor] != '\n') cursor++;
                        return makeToken(MISC_LINE_COMMENT, origin, cursor);
                    }
                    else if (fileContent[cursor + 1] == '*') {
                        //Block comment: emit MISC_BLOCK_COMMENT trivia token (preserved for CST/IDE).
                        //Payload includes the leading "/*" and the trailing "*/" if present;
                        //if the comment is unterminated, payload covers everything to EOF.
                        cursor += 2;
                        while (cursor + 1 < fileContent.size() && !(fileContent[cursor] == '*' && fileContent[cursor + 1] == '/')) cursor++;
                        //consume the closing "*/"
                        if (cursor + 1 < fileContent.size()) cursor += 2;
                        //unterminated
                        else cursor = fileContent.size();
                        return makeToken(MISC_BLOCK_COMMENT, origin, cursor);
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

    inline void parse() noexcept {
        Token token{};
        do {
            token = getNextToken();
            tokens.push_back(token);
        } while (token.type != TokenType::MISC_EOF);
    }
}