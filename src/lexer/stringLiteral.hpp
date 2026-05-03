#pragma once
#include <utility>

#include "../currentFile.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint8_t u8;
    typedef uint64_t u64;
    using std::to_underlying;
    using enum TokenType;
    using enum StringTypeFlags;

    [[nodiscard]] inline bool shouldBeConsideredStringLiteral(u64 cursor) noexcept {
        for (u64 i = cursor; i < fileContent.size(); i++) switch (fileContent[i]) {
            case 'c': case 'r': case 'f': case '#': break;
            case '"': return true;
            default: return false;
        }
        return false;
    }

    [[nodiscard]] inline Token getStringLiteral(u64& cursor) noexcept {
        const u64 origin = cursor;
        u64 fenceCount = 0;
        StringType strlType;
        bool invalid = false, gotOpeningQuote = false;
        for (; cursor < fileContent.size(); cursor++) {
            switch (fileContent[cursor]) {
                case 'c':
                    if (strlType.get(StringTypeFlags::CStyle)) invalid = true;
                    else strlType.set(StringTypeFlags::CStyle); 
                    break;
                case 'r':
                    if (strlType.get(StringTypeFlags::Raw)) invalid = true;
                    else strlType.set(StringTypeFlags::Raw);
                    break;
                case 'f':
                    if (strlType.get(StringTypeFlags::Format)) invalid = true;
                    else strlType.set(StringTypeFlags::Format);
                    break;
                case '#':
                    if (!strlType.get(StringTypeFlags::Raw)) invalid = true;
                    fenceCount++;
                    break;
                case '"':
                    gotOpeningQuote = true;
                    break;
                default: //This should never happen, but just in case, we instantly return an error token.
                    cursor++;
                    return {.type = MISC_ERROR, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
            if (gotOpeningQuote) {
                if (strlType.get(StringTypeFlags::CStyle) && strlType.get(StringTypeFlags::Format)) invalid = true;
                cursor++;
                break;
            }
        }
        bool escaping = false, tryingToCloseFence = false;
        u64 tempFenceCount = 0;
        for (; cursor < fileContent.size(); cursor++) {
            if (
                !strlType.get(StringTypeFlags::Raw)
             && (fileContent[cursor] == '\n' || fileContent[cursor] == '\r')) {
                //Unescaped newlines are not allowed in non-raw string literals.
                return {.type = MISC_ERROR, .strlType = strlType, .payload = span(fileContent.data() + origin, cursor - origin)};
            }
            if (fileContent[cursor] == '#') {
                if (strlType.get(StringTypeFlags::Raw) && tryingToCloseFence) {
                    tempFenceCount++;
                    if (tempFenceCount == fenceCount) {
                        cursor++;
                        return {.type = invalid ? MISC_ERROR : LT_STRING, .strlType = strlType, .payload = span(fileContent.data() + origin, cursor - origin)};
                    }
                }
            }
            else {
                tryingToCloseFence = false;
                tempFenceCount = 0;
            }
            if (fileContent[cursor] == '"') {
                if (strlType.get(StringTypeFlags::Raw)) {
                    if (fenceCount == 0) {
                        cursor++;
                        return {.type = invalid ? MISC_ERROR : LT_STRING, .strlType = strlType, .payload = span(fileContent.data() + origin, cursor - origin)};
                    }
                    else if (tryingToCloseFence) {
                        tryingToCloseFence = false;
                        tempFenceCount = 0;
                    }
                    else tryingToCloseFence = true;
                }
                else if (!escaping) {
                    cursor++;
                    return {.type = invalid ? MISC_ERROR : LT_STRING, .strlType = strlType, .payload = span(fileContent.data() + origin, cursor - origin)};
                }
            }
            if (fileContent[cursor] == '\\') {
                escaping = !escaping;
                strlType.set(StringTypeFlags::Escaped);
            }
            else escaping = false;
        }
        //EOF reached.
        return {.type = MISC_ERROR, .strlType = strlType, .payload = span(fileContent.data() + origin, cursor - origin)};
    }
}