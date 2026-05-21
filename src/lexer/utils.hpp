#pragma once
#include <span>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer::internal {
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

    [[nodiscard]] inline bool startsWith(const u64& cursor, u8 first, u8 second) noexcept {
        return cursor + 1 < fileContent.size() && fileContent[cursor] == first && fileContent[cursor + 1] == second;
    }

    [[nodiscard]] inline bool startsWithBom(const u64& cursor) noexcept {
        return cursor == 0 && fileContent.size() >= 3 && fileContent[0] == 239 && fileContent[1] == 187 && fileContent[2] == 191;
    }

    [[nodiscard]] inline bool isHorizontalWhitespace(u8 c) noexcept {
        return c == ' ' || c == '\t';
    }

    [[nodiscard]] inline bool startsNewline(const u64& cursor) noexcept {
        return cursor < fileContent.size() && (fileContent[cursor] == '\n' || fileContent[cursor] == '\r');
    }

    inline void consumeNewline(u64& cursor) noexcept {
        if (cursor < fileContent.size() && fileContent[cursor] == '\r' && cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '\n') cursor += 2;
        else if (cursor < fileContent.size()) cursor++;
    }

    inline void consumeLineComment(u64& cursor) noexcept {
        cursor += 2;
        while (cursor < fileContent.size() && fileContent[cursor] != '\n' && fileContent[cursor] != '\r') cursor++;
    }

    [[nodiscard]] inline bool consumeBlockComment(u64& cursor) noexcept {
        bool containsNewline = false;
        cursor += 2;
        while (cursor + 1 < fileContent.size() && !(fileContent[cursor] == '*' && fileContent[cursor + 1] == '/')) {
            if (fileContent[cursor] == '\r') {
                containsNewline = true;
                if (cursor + 1 < fileContent.size() && fileContent[cursor + 1] == '\n') cursor += 2;
                else cursor++;
            }
            else {
                if (fileContent[cursor] == '\n') containsNewline = true;
                cursor++;
            }
        }
        if (cursor + 1 < fileContent.size()) cursor += 2;
        else cursor = fileContent.size();
        return containsNewline;
    }

    [[nodiscard]] inline bool isWhitespace(char c) noexcept {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    [[nodiscard]] inline bool startsTrivia(const u64& cursor) noexcept {
        return cursor < fileContent.size() && (isWhitespace(fileContent[cursor]) || startsWith(cursor, '/', '/') || startsWith(cursor, '/', '*') || startsWithBom(cursor));
    }

    [[nodiscard]] inline span<const u8> scanLeadingTrivia(u64& cursor) noexcept {
        const u64 origin = cursor;
        while (startsTrivia(cursor)) {
            if (startsWithBom(cursor)) cursor += 3;
            else if (isWhitespace(fileContent[cursor])) cursor++;
            else if (startsWith(cursor, '/', '/')) consumeLineComment(cursor);
            else if (startsWith(cursor, '/', '*')) (void)consumeBlockComment(cursor);
        }
        return makePayload(origin, cursor);
    }

    [[nodiscard]] inline span<const u8> scanTrailingTrivia(u64& cursor) noexcept {
        const u64 origin = cursor;
        while (cursor < fileContent.size()) {
            if (isHorizontalWhitespace(fileContent[cursor])) cursor++;
            else if (startsWith(cursor, '/', '/')) consumeLineComment(cursor);
            else if (startsNewline(cursor)) {
                consumeNewline(cursor);
                break;
            }
            else if (startsWith(cursor, '/', '*')) {
                const bool containsNewline = consumeBlockComment(cursor);
                if (containsNewline) break;
            }
            else break;
        }
        return makePayload(origin, cursor);
    }

    [[nodiscard]] inline bool isValidIdentifierStart(char c) noexcept {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    [[nodiscard]] inline bool isValidIdentifierPart(char c) noexcept {
        return isValidIdentifierStart(c) || (c >= '0' && c <= '9');
    }

    [[nodiscard]] inline bool isHexDigit(char c) noexcept {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    [[nodiscard]] inline u8 getHexValue(char c) noexcept {
        if (c >= '0' && c <= '9') return c - '0';
        else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        else return 0; //Should not happen if we only call this on valid hex digits.
    }

    [[nodiscard]] inline bool isOctalDigit(char c) noexcept {
        return c >= '0' && c <= '7';
    }
}