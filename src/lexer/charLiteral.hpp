#pragma once
#include <array>
#include <limits>
#include <span>

#include "../currentFile.hpp"
#include "../util/string.hpp"
#include "tokens.hpp"
#include "utils.hpp"

namespace AOO::Lexer {
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    using std::array, std::span, std::numeric_limits, Util::isValidIdentifierStart, Util::isValidIdentifierPart, Util::isHexDigit, Util::getHexValue, Util::isOctalDigit;
    using enum TokenType;

    [[nodiscard]] inline Token getOctalEscapeSequence(u64& cursor, u8 secondChar) noexcept {
        u8 secondOctalDigit, thirdOctalDigit;

        //To the second octal digit's place.
        cursor += 3;

        //We have a second octal digit.
        if (isOctalDigit(fileContent[cursor]) && cursor + 1 < fileContent.size()) secondOctalDigit = fileContent[cursor];
        //Stop with one digit.
        else if (fileContent[cursor] == '\'') {
            cursor++;
            return {.type = LT_CHAR, .charPayload = static_cast<u8>(secondChar - '0')};
        }
        //Invalid.
        else {
            cursor++;
            return greedyUntilAndErrorOut(cursor, '\'', cursor - 4);
        }

        //To the third octal digit's place.
        cursor++;

        //We have a third octal digit.
        if (isOctalDigit(fileContent[cursor]) && cursor + 1 < fileContent.size()) {
            thirdOctalDigit = fileContent[cursor];
            if (fileContent[cursor + 1] == '\'') {
                cursor += 2;
                const u16 value = (secondChar - '0') * 64 + (secondOctalDigit - '0') * 8 + (thirdOctalDigit - '0');
                if (value > 255) return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 6, 6)};
                return {.type = LT_CHAR, .charPayload = static_cast<u8>(value)};
            }
            else {
                //More than three octal digits is not allowed, we need to greedy until ' and error out.
                cursor += 2;
                return greedyUntilAndErrorOut(cursor, '\'', cursor - 6);
            }
        }
        //Stop with two digits.
        else if (fileContent[cursor] == '\'') {
            cursor++;
            //Two digits never overflow.
            return {.type = LT_CHAR, .charPayload = static_cast<u8>((secondChar - '0') * 8 + (secondOctalDigit - '0'))};
        }
        //Invalid. Greedy until ' and error out.
        else {
            cursor++;
            return greedyUntilAndErrorOut(cursor, '\'', cursor - 5);
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
                        return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
                    }
                    else if (cursor == origin + 4) return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, 4)};
                    else return {.type = LT_CHAR, .charPayload = static_cast<u8>(value)};
                }
                else return greedyUntilAndErrorOut(cursor, '\'', origin);
            }
            else /*if (outOfRange)*/ return greedyUntilAndErrorOut(cursor, '\'', origin);
        }
        //File ended.
        else return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
    }

    //Recognizes '\u{HHHH}' (exactly 4 hex digits) and '\u{HHHHHHHH}' (exactly 8 hex digits).
    //On entry: cursor points at the opening apostrophe; cursor[0]='\'', cursor[1]='\\', cursor[2]='u'.
    [[nodiscard]] inline Token getUnicodeEscapeSequence(u64& cursor) noexcept {
        const u64 origin = cursor;
        cursor += 3; //Past '\'', '\\', 'u'.

        //Expect '{'.
        if (cursor >= fileContent.size() || fileContent[cursor] != '{') {
            return greedyUntilAndErrorOut(cursor, '\'', origin);
        }
        cursor++;

        //Read up to 8 hex digits.
        u64 value = 0;
        u64 digitCount = 0;
        while (cursor < fileContent.size() && isHexDigit(fileContent[cursor])) {
            if (digitCount >= 8) {
                //More than 8 hex digits.
                return greedyUntilAndErrorOut(cursor, '\'', origin);
            }
            value = (value << 4) | static_cast<u64>(getHexValue(fileContent[cursor]));
            digitCount++;
            cursor++;
        }
        //Only exactly 4 or exactly 8 hex digits are accepted.
        if (digitCount != 4 && digitCount != 8) {
            return greedyUntilAndErrorOut(cursor, '\'', origin);
        }

        //Expect '}'.
        if (cursor >= fileContent.size() || fileContent[cursor] != '}') {
            return greedyUntilAndErrorOut(cursor, '\'', origin);
        }
        cursor++;

        //Expect closing '\''.
        if (cursor >= fileContent.size() || fileContent[cursor] != '\'') {
            return greedyUntilAndErrorOut(cursor, '\'', origin);
        }
        cursor++;

        //Char literals hold a single byte; values > 255 are out of range.
        if (value > 255) return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
        return {.type = LT_CHAR, .charPayload = static_cast<u8>(value)};
    }

    [[nodiscard]] inline Token getValidEscapedChar(u64& cursor, u8 escapedChar) noexcept {
        cursor += 3;
        if (fileContent[cursor] == '\'') {
            cursor++;
            return {.type = LT_CHAR, .charPayload = escapedChar};
        }
        else {
            cursor++;
            return greedyUntilAndErrorOut(cursor, '\'', cursor - 4);
        }
    }

    //You may get a LT_LABEL from this.
    [[nodiscard]] inline Token getCharLiteralOrLabel(u64& cursor) noexcept {
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
                    case 'u': return getUnicodeEscapeSequence(cursor);
                    //Greedy until ' and error out.
                    default: {
                        cursor += 3;
                        return greedyUntilAndErrorOut(cursor, '\'', cursor - 3);
                    }
                }
            }
            //Not enough chars to be escaped. Just consume all characters till the end.
            else if (firstChar == '\\') {
                cursor += 3;
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 3, 3)};
            }
            //Empty char literal is invalid.
            else if (firstChar == '\'') {
                cursor += 2;
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 2, 2)};
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
                                return {.type = LT_LABEL, .payload = span(fileContent.data() + origin + 1, cursor - origin - 2)};
                            }
                            else if (!isValidIdentifierPart(fileContent[cursor])) possibleLabel = false;
                        }
                        if (fileContent[cursor] == '\'') {
                            cursor++;
                            return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
                        }
                        cursor++;
                    }
                    //File ended.
                    return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
                else { //Closed -> char literal.
                    cursor += 3;
                    return {.type = LT_CHAR, .charPayload = firstChar};
                }
            }
        }
        else {
            if (cursor + 1 < fileContent.size()) {
                //Consume the last 2 characters.
                cursor += 2;
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 2, 2)};
            }
            else {
                //Consume the last '.
                cursor++;
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 1, 1)};
            }
        }
    }
}