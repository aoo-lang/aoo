#pragma once
#include <span>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint64_t u64;
    using std::span;
    using enum TokenType;

    [[nodiscard]] inline span<const u8> makePayload(u64 origin, u64 cursor) noexcept {
        const u8* begin = fileContent.empty() ? nullptr : fileContent.data() + origin;
        return span<const u8>(begin, cursor - origin);
    }

    [[nodiscard]] inline Token makeToken(TokenType type, u64 origin, u64 cursor) noexcept {
        return {.type = type, .payload = makePayload(origin, cursor)};
    }

    [[nodiscard]] inline Token greedyUntilAndErrorOut(u64& cursor, u8 delimiter, u64 origin) noexcept {
        while (cursor < fileContent.size() && fileContent[cursor] != delimiter) cursor++;
        //If we stopped because we found the delimiter, consume it. We cannot flip the condition because fileContent[cursor] might be out of bounds and crash the lexer.
        if (cursor < fileContent.size()) cursor++;
        return makeToken(MISC_ERROR, origin, cursor);
    }

    [[nodiscard]] inline bool getCommonEscapeValue(u8 c, u8& result) noexcept {
        switch (c) {
            case 'a':  result = '\a'; return true;
            case 'b':  result = '\b'; return true;
            case 'f':  result = '\f'; return true;
            case 'n':  result = '\n'; return true;
            case 'r':  result = '\r'; return true;
            case 't':  result = '\t'; return true;
            case 'v':  result = '\v'; return true;
            case '\\': result = '\\'; return true;
            case '\'': result = '\''; return true;
            case '"':  result = '"';  return true;
            case '?':  result = '?';  return true;
            default:                  return false;
        }
    }
}