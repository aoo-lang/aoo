#pragma once
#include <span>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint64_t u64;
    using std::span;
    using enum TokenType;

    [[nodiscard]] inline Token greedyUntilAndErrorOut(u64& cursor, u8 delimiter, u64 origin) noexcept {
        while (cursor < fileContent.size() && fileContent[cursor] != delimiter) cursor++;
        //If we stopped because we found the delimiter, consume it. We cannot flip the condition because fileContent[cursor] might be out of bounds and crash the lexer.
        if (cursor < fileContent.size()) cursor++;
        return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
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