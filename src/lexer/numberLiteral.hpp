#pragma once

#include "../currentFile.hpp"
#include "../util/string.hpp"
#include "tokens.hpp"

namespace AO::Lexer {
    typedef uint64_t u64;
    using Util::isWhitespace;
    using enum TokenType;

    [[nodiscard]] inline Token decimal(u64& cursor, u64 start) noexcept {
        
    }

    [[nodiscard]] inline Token hex(u64& cursor, u64 start) noexcept {

    }

    [[nodiscard]] inline Token binary(u64& cursor, u64 start) noexcept {

    }

    [[nodiscard]] inline Token octal(u64& cursor, u64 start) noexcept {

    }

    [[nodiscard]] inline Token getNumberLiteral(u64& cursor) noexcept {
        using enum TokenType;
        using enum StringType;

        //Temp
        cursor++;
        return {.type = GN_U8, .strType = NotAString, .u8Payload = 0};
        //End Temp

        const u64 start = cursor;
        if (fileContent[cursor] == '0') {
            cursor++;
            if (fileContent[cursor] == 'x' || fileContent[cursor] == 'X') {
                cursor++;
                return hex(cursor, start);
            }
            else if (fileContent[cursor] == 'b' || fileContent[cursor] == 'B') {
                cursor++;
                return binary(cursor, start);
            }
            else if (fileContent[cursor] == 'o' || fileContent[cursor] == 'O') {
                cursor++;
                return octal(cursor, start);
            }
            //We just reject leading zeros altogether so we don't need to worry about the octal hobby.
            else return {.type = MISC_ERROR, .strType = NotAString, .payload = {}};
        }
        return decimal(cursor, start);
    }
}