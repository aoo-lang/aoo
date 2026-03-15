#pragma once
#include <array>
#include <span>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AO::Lexer {
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint64_t u64;
    using std::array, std::span;
    using enum TokenType;
    using enum StringType;

    [[nodiscard]] inline Token getOctalEscapeSequence(u64& cursor, u8 secondChar) noexcept {
        u8 secondOctalDigit, thirdOctalDigit;

        //To the second octal digit's place.
        cursor += 3;

        //We have a second octal digit.
        if (fileContent[cursor] >= '0' && fileContent[cursor] <= '7' && cursor + 1 < fileContent.size()) secondOctalDigit = fileContent[cursor];
        //Stop with one digit.
        else if (fileContent[cursor] == '\'') {
            cursor++;
            return {.type = GN_CHAR, .strType = NotAString, .u8Payload = static_cast<u8>(secondChar - '0')};
        }
        //Invalid. Greedy until ' and error out.
        else {
            cursor++;
            const u64 origin = cursor - 4;
            while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
            //If we stopped because we found a closing ', consume it.
            if (fileContent[cursor] == '\'') cursor++;
            return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
        }

        //To the third octal digit's place.
        cursor++;

        //We have a third octal digit.
        if (fileContent[cursor] >= '0' && fileContent[cursor] <= '7' && cursor + 1 < fileContent.size()) {
            thirdOctalDigit = fileContent[cursor];
            if (fileContent[cursor + 1] == '\'') {
                cursor += 2;
                const u16 value = (secondChar - '0') * 64 + (secondOctalDigit - '0') * 8 + (thirdOctalDigit - '0');
                if (value > 255) return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 6, 6)};
                return {.type = GN_CHAR, .strType = NotAString, .u8Payload = static_cast<u8>(value)};
            }
            else {
                //More than three octal digits is not allowed, we need to greedy until ' and error out.
                cursor += 2;
                const u64 origin = cursor - 6;
                while(cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                //If we stopped because we found a closing ', consume it.
                if (fileContent[cursor] == '\'') cursor++;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
        }
        //Stop with two digits.
        else if (fileContent[cursor] == '\'') {
            cursor++;
            //Two digits never overflow.
            return {.type = GN_CHAR, .strType = NotAString, .u8Payload = static_cast<u8>((secondChar - '0') * 8 + (secondOctalDigit - '0'))};
        }
        //Invalid. Greedy until ' and error out.
        else {
            cursor++;
            const u64 origin = cursor - 5;
            while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
            //If we stopped because we found a closing ', consume it.
            if (fileContent[cursor] == '\'') cursor++;
            return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
        }
    }

    [[nodiscard]] inline Token getHexEscapeSequence(u64& cursor) noexcept {

    }

    [[nodiscard]] inline Token getCharLiteral(u64& cursor) noexcept {
        if (cursor + 2 < fileContent.size()) {
            const u8 firstChar = fileContent[cursor + 1];
            if (firstChar == '\\' && cursor + 3 < fileContent.size()) {
                //maybe escape sequence
                const u8 secondChar = fileContent[cursor + 2];
                switch (secondChar) {
                    case 'a': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\a'};
                    case 'b': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\b'};
                    case 'f': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\f'};
                    case 'n': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\n'};
                    case 'r': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\r'};
                    case 't': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\t'};
                    case 'v': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\v'};
                    case '\\': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\\'};
                    case '\'': cursor += 4; return {.type = GN_CHAR, .strType = NotAString, .u8Payload = '\''};
                    //Octal escape sequences.
                    case '0'...'7': return getOctalEscapeSequence(cursor, secondChar);
                    //Hexadecimal escape sequences.
                    case 'x': return getHexEscapeSequence(cursor);
                    //Greedy until ' and error out.
                    default:
                }
            }
            //Not enough chars to be escaped. Greedy until the end.
            else if (firstChar == '\\') {

            }
            else { //Normal char literal
                //Not closed. Greedy until '.
                if (fileContent[cursor + 2] != '\'') {

                }
                cursor += 3;
                return {.type = GN_CHAR, .strType = NotAString, .u8Payload = firstChar};
            }
        }
        else {
            cursor++;
            return {.type = CH_APOSTROPHE, .strType = NotAString, .payload = {}};
        }
    }
}