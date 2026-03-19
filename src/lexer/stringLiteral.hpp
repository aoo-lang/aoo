#pragma once

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {

    [[nodiscard]] inline Token getStringLiteral(u64& cursor) noexcept {
        using enum TokenType;
        using enum StringType;

        cursor++;
        const u64 stringStart = cursor;
        while (
            cursor < fileContent.size()
         && fileContent[cursor] != '"' && fileContent[cursor] != '\n' && fileContent[cursor] != '\r'
        ) cursor++;
        if (cursor == fileContent.size() || fileContent[cursor] == '\n' || fileContent[cursor] == '\r') {
            //Unterminated string literal
            return {.type = MISC_ERROR, .strType = NotAString, .payload = {}};
        }
        cursor++;
        return {.type = GN_STRING, .strType = Normal, .payload = span<const u8>(fileContent.data() + stringStart, cursor - stringStart - 1)};
    }
}