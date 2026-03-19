#pragma once

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    using enum TokenType;
    using enum StringType;

    namespace detail {
        [[nodiscard]] inline Token getStringLiteralTyped(u64& cursor, StringType strType, u64 origin) noexcept {
            cursor++;
            while (cursor < fileContent.size()) {
                cursor++;
                if (fileContent[cursor] == '"' && fileContent[cursor - 1] != '\\') {
                    cursor++;
                    return {.type = GN_STRING, .strType = strType, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
            }
            //File ended before closing quote.
            return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
        }
    }

    [[nodiscard]] inline Token getStringLiteral(u64& cursor) noexcept {
        const u64 origin = cursor;
        switch (fileContent[cursor]) {
            case 'b': cursor++; return detail::getStringLiteralTyped(cursor, Byte, origin);
            case 'c': cursor++; return detail::getStringLiteralTyped(cursor, CStyle, origin);
            case 'r': cursor++; return detail::getStringLiteralTyped(cursor, Raw, origin);
            case 'f': cursor++; return detail::getStringLiteralTyped(cursor, Format, origin);
            case '"': return detail::getStringLiteralTyped(cursor, Normal, origin);
            default: //This should never happen, but just in case, we return an error token.
                cursor++;
                return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, 1)};
        }
    }
}