#pragma once
#include <array>
#include <limits>
#include <span>

#include "../currentFile.hpp"
#include "../util/string.hpp"
#include "tokens.hpp"

namespace AO::Lexer {
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    using std::array, std::span, std::numeric_limits, Util::isValidIdentifierStart, Util::isValidIdentifierPart, Util::isHexDigit, Util::getHexValue, Util::isOctalDigit;
    using enum TokenType;
    using enum StringType;

    [[nodiscard]] inline Token getOctalEscapeSequence(u64& cursor, u8 secondChar) noexcept {
        u8 secondOctalDigit, thirdOctalDigit;

        //To the second octal digit's place.
        cursor += 3;

        //We have a second octal digit.
        if (isOctalDigit(fileContent[cursor]) && cursor + 1 < fileContent.size()) secondOctalDigit = fileContent[cursor];
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
            if (cursor < fileContent.size()) cursor++;
            return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
        }

        //To the third octal digit's place.
        cursor++;

        //We have a third octal digit.
        if (isOctalDigit(fileContent[cursor]) && cursor + 1 < fileContent.size()) {
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
            if (cursor < fileContent.size()) cursor++;
            return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
        }
    }

    [[nodiscard]] inline Token getHexEscapeSequence(u64& cursor) noexcept {
        cursor += 3;
        const u64 origin = cursor - 3;
        u64 value = 0;
        //bool outOfRange = false;
        while (cursor < fileContent.size() && isHexDigit(fileContent[cursor])) {
            if (value > (numeric_limits<u64>::max() >> 4)) {
                //outOfRange = true;
                break;
            }
            value = value << 4 | static_cast<u64>(getHexValue(fileContent[cursor]));
            cursor++;
        }
        if (cursor < fileContent.size()) {
            if (!isHexDigit(fileContent[cursor])) {
                if (fileContent[cursor] == '\'') {
                    cursor++;
                    if (value > 255) {
                        return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                    }
                    else if (cursor == origin + 4) return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, 4)};
                    else return {.type = GN_CHAR, .strType = NotAString, .u8Payload = static_cast<u8>(value)};
                }
                else { //Greedy until ' and error out.
                    while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                    //If we stopped because we found a closing ', consume it.
                    if (cursor < fileContent.size()) cursor++;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
            }
            else /*if (outOfRange)*/ { //Greedy until ' and error out.
                while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                //If we stopped because we found a closing ', consume it.
                if (cursor < fileContent.size()) cursor++;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
        }
        //File ended.
        else return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
    }

    [[nodiscard]] inline Token getUnicodeEscapeSequence(u64& cursor) noexcept {
        cursor += 2;
        if (fileContent[cursor] == 'u') {
            if (cursor + 5 < fileContent.size()) {
                cursor++;
                if (isHexDigit(fileContent[cursor]) && isHexDigit(fileContent[cursor + 1]) && isHexDigit(fileContent[cursor + 2]) && isHexDigit(fileContent[cursor + 3]) && fileContent[cursor + 4] == '\'') {
                    const u16 value = static_cast<u16>(getHexValue(fileContent[cursor]) << 12) | static_cast<u16>(getHexValue(fileContent[cursor + 1]) << 8) | static_cast<u16>(getHexValue(fileContent[cursor + 2]) << 4) | static_cast<u16>(getHexValue(fileContent[cursor + 3]));
                    cursor += 5;
                    if (value > 255) return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 8, 8)};
                    else return {.type = GN_CHAR, .strType = NotAString, .u8Payload = static_cast<u8>(value)};
                }
                //Invalid hex digits. Stop immediately! We cannot greedy by the stupid standard. Yes, we greedy on octal escape sequences when we encounter invalid digits but we don't on hex ones. How consistent.
                else if (!isHexDigit(fileContent[cursor])) {
                    cursor++;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 4, 4)};
                }
                else if (!isHexDigit(fileContent[cursor + 1])) {
                    cursor += 2;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 5, 5)};
                }
                else if (!isHexDigit(fileContent[cursor + 2])) {
                    cursor += 3;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 6, 6)};
                }
                else if (!isHexDigit(fileContent[cursor + 3])) {
                    cursor += 4;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 7, 7)};
                }
                //Not ended after 4 hex digits. Greedy until ' and error out.
                else /*if (fileContent[cursor + 4] != '\'')*/ {
                    cursor += 5;
                    const u64 origin = cursor - 8;
                    while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                    //If we stopped because we found a closing ', consume it.
                    if (cursor < fileContent.size()) cursor++;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
            }
            else {
                cursor++;
                const u64 origin = cursor - 3;
                while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                //If we stopped because we found a closing ', consume it.
                if (cursor < fileContent.size()) cursor++;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
        }
        else /*if (fileContent[cursor] == 'U')*/ {
            if (cursor + 9 < fileContent.size()) {
                cursor++;
                if (
                    isHexDigit(fileContent[cursor])
                 && isHexDigit(fileContent[cursor + 1])
                 && isHexDigit(fileContent[cursor + 2])
                 && isHexDigit(fileContent[cursor + 3])
                 && isHexDigit(fileContent[cursor + 4])
                 && isHexDigit(fileContent[cursor + 5])
                 && isHexDigit(fileContent[cursor + 6])
                 && isHexDigit(fileContent[cursor + 7])
                 && fileContent[cursor + 8] == '\''
                ) {
                    const u32 value = static_cast<u32>(getHexValue(fileContent[cursor]) << 28) | static_cast<u32>(getHexValue(fileContent[cursor + 1]) << 24) | static_cast<u32>(getHexValue(fileContent[cursor + 2]) << 20) | static_cast<u32>(getHexValue(fileContent[cursor + 3]) << 16) | static_cast<u32>(getHexValue(fileContent[cursor + 4]) << 12) | static_cast<u32>(getHexValue(fileContent[cursor + 5]) << 8) | static_cast<u32>(getHexValue(fileContent[cursor + 6]) << 4) | static_cast<u32>(getHexValue(fileContent[cursor + 7]));
                    cursor += 9;
                    if (value > 255) return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 12, 12)};
                    else return {.type = GN_CHAR, .strType = NotAString, .u8Payload = static_cast<u8>(value)};
                }
                //Invalid hex digits. Stop immediately! We cannot greedy by the stupid standard. Yes, we greedy on octal escape sequences when we encounter invalid digits but we don't on hex ones. How consistent.
                else if (!isHexDigit(fileContent[cursor])) {
                    cursor++;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 4, 4)};
                }
                else if (!isHexDigit(fileContent[cursor + 1])) {
                    cursor += 2;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 5, 5)};
                }
                else if (!isHexDigit(fileContent[cursor + 2])) {
                    cursor += 3;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 6, 6)};
                }
                else if (!isHexDigit(fileContent[cursor + 3])) {
                    cursor += 4;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 7, 7)};
                }
                else if (!isHexDigit(fileContent[cursor + 4])) {
                    cursor += 5;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 8, 8)};
                }
                else if (!isHexDigit(fileContent[cursor + 5])) {
                    cursor += 6;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 9, 9)};
                }
                else if (!isHexDigit(fileContent[cursor + 6])) {
                    cursor += 7;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 10, 10)};
                }
                else if (!isHexDigit(fileContent[cursor + 7])) {
                    cursor += 8;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 11, 11)};
                }
                else if (!isHexDigit(fileContent[cursor + 8])) {
                    cursor += 9;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 12, 12)};
                }
                //Not ended after 4 hex digits. Greedy until ' and error out.
                else /*if (fileContent[cursor + 8] != '\'')*/ {
                    cursor += 9;
                    const u64 origin = cursor - 12;
                    while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                    //If we stopped because we found a closing ', consume it.
                    if (cursor < fileContent.size()) cursor++;
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
            }
            else {
                cursor++;
                const u64 origin = cursor - 3;
                while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                //If we stopped because we found a closing ', consume it.
                if (cursor < fileContent.size()) cursor++;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
        }
    }

    [[nodiscard]] inline Token getValidEscapedChar(u64& cursor, u8 escapedChar) noexcept {
        cursor += 3;
        if (fileContent[cursor] == '\'') {
            cursor++;
            return {.type = GN_CHAR, .strType = NotAString, .u8Payload = escapedChar};
        }
        else {
            cursor++;
            const u64 origin = cursor - 4;
            while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
            //If we stopped because we found a closing ', consume it.
            if (cursor < fileContent.size()) cursor++;
            return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
        }
    }

    //You may get a GN_LABEL from this.
    [[nodiscard]] inline Token getCharLiteral(u64& cursor) noexcept {
        if (cursor + 2 < fileContent.size()) {
            const u8 firstChar = fileContent[cursor + 1];
            if (firstChar == '\\' && cursor + 3 < fileContent.size()) {
                //maybe escape sequence
                const u8 secondChar = fileContent[cursor + 2];
                switch (secondChar) {
                    case 'a': return getValidEscapedChar(cursor, '\a');
                    case 'b': return getValidEscapedChar(cursor, '\b');
                    case 'f': return getValidEscapedChar(cursor, '\f');
                    case 'n': return getValidEscapedChar(cursor, '\n');
                    case 'r': return getValidEscapedChar(cursor, '\r');
                    case 't': return getValidEscapedChar(cursor, '\t');
                    case 'v': return getValidEscapedChar(cursor, '\v');
                    case '\\': return getValidEscapedChar(cursor, '\\');
                    case '\'': return getValidEscapedChar(cursor, '\'');
                    case '?': return getValidEscapedChar(cursor, '?');
                    //Octal escape sequences.
                    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': return getOctalEscapeSequence(cursor, secondChar);
                    //Hexadecimal escape sequences.
                    case 'x': return getHexEscapeSequence(cursor);
                    //Unicode escape sequences.
                    case 'u': case 'U': return getUnicodeEscapeSequence(cursor);
                    //Greedy until ' and error out.
                    default: {
                        cursor += 3;
                        const u64 origin = cursor - 3;
                        while (cursor < fileContent.size() && fileContent[cursor] != '\'') cursor++;
                        //If we stopped because we found a closing ', consume it.
                        if (cursor < fileContent.size()) cursor++;
                        return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                    }
                }
            }
            //Not enough chars to be escaped. Just consume all characters till the end.
            else if (firstChar == '\\') {
                cursor += 3;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 3, 3)};
            }
            //Empty char literal is invalid.
            else if (firstChar == '\'') {
                cursor += 2;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 2, 2)};
            }
            else { //Normal char literal or label
                //Not closed. Check for : for labels, otherwise greedy until ' and error out.
                if (fileContent[cursor + 2] != '\'') {
                    const u64 origin = cursor;
                    cursor++;
                    bool possibleLabel = isValidIdentifierStart(fileContent[cursor]);
                    cursor++;
                    while (cursor < fileContent.size()) {
                        if (possibleLabel) {
                            if (fileContent[cursor] == ':') {
                                cursor++;
                                //'_: is not a valid label because a single _ is not a valid identifier.
                                if (cursor == origin + 3 && fileContent[cursor - 2] == '_') return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, 3)};
                                return {.type = GN_LABEL, .strType = NotAString, .payload = span(fileContent.data() + origin + 1, cursor - origin - 2)};
                            }
                            else if (!isValidIdentifierPart(fileContent[cursor])) possibleLabel = false;
                        }
                        if (fileContent[cursor] == '\'') {
                            cursor++;
                            return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                        }
                        cursor++;
                    }
                    //File ended.
                    return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
                else { //Closed -> char literal.
                    cursor += 3;
                    return {.type = GN_CHAR, .strType = NotAString, .u8Payload = firstChar};
                }
            }
        }
        else {
            if (cursor + 1 < fileContent.size()) {
                //Consume the last 2 characters.
                cursor += 2;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 2, 2)};
            }
            else {
                //Consume the last '.
                cursor++;
                return {.type = MISC_ERROR, .strType = NotAString, .payload = span(fileContent.data() + cursor - 1, 1)};
            }
        }
    }
}