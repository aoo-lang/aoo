#pragma once

#include "../currentFile.hpp"
#include "../util/cmp.hpp"
#include "../util/string.hpp"
#include "stringLiteral.hpp"
#include "tokens.hpp"

namespace AOO::Lexer {
    typedef uint64_t u64;
    using Util::equals, Util::isValidIdentifierStart, Util::isValidIdentifierPart;
    using enum TokenType;

    namespace detail {
        [[nodiscard]] inline TokenType detectKeyword(const span<const u8> str) noexcept {
            if (equals(str, "module")) return KW_MODULE;
            else if (equals(str, "import")) return KW_IMPORT;
            else if (equals(str, "export")) return KW_EXPORT;
            else if (equals(str, "type")) return KW_TYPE;
            else if (equals(str, "trait")) return KW_TRAIT;
            else if (equals(str, "enum")) return KW_ENUM;
            //-----------------------------
            else if (equals(str, "if")) return KW_IF;
            else if (equals(str, "else")) return KW_ELSE;
            else if (equals(str, "for")) return KW_FOR;
            else if (equals(str, "break")) return KW_BREAK;
            else if (equals(str, "continue")) return KW_CONTINUE;
            else if (equals(str, "match")) return KW_MATCH;
            else if (equals(str, "return")) return KW_RETURN;
            //-----------------------------
            else if (equals(str, "public")) return KW_PUBLIC;
            else if (equals(str, "private")) return KW_PRIVATE;
            else if (equals(str, "self")) return KW_SELF;
            else if (equals(str, "op")) return KW_OP;
            //-----------------------------
            else if (equals(str, "void")) return KW_VOID;
            else if (equals(str, "auto")) return KW_AUTO;
            else if (equals(str, "as")) return KW_AS;
            else if (equals(str, "val")) return KW_VAL;
            else if (equals(str, "ref")) return KW_REF;
            //-----------------------------
            else if (equals(str, "in")) return KW_IN;
            else if (equals(str, "dup")) return KW_DUP;
            else return LT_IDENTIFIER;
        }
    }

    [[nodiscard]] inline Token getIdentifierLike(u64& cursor) noexcept {
        switch (fileContent[cursor]) {
            case '#':
                //Compiler identifier
                break;
            case '@':
                //Macros
                break;
            case '$':
                //Macro parameters
                break;
            case '`':
                //AST storage identifiers
                break;
        }
        if (!isValidIdentifierStart(fileContent[cursor])) {
            cursor++;
            return {.type = MISC_ERROR, .payload = span(fileContent.data() + cursor - 1, 1)};
        }
        if ((
            fileContent[cursor] == 'c'
         || fileContent[cursor] == 'r'
         || fileContent[cursor] == 'f'
        ) && cursor + 1 < fileContent.size() && shouldBeConsideredStringLiteral(cursor)) return getStringLiteral(cursor);
        const u64 origin = cursor;
        do { cursor++; } while (cursor < fileContent.size() && isValidIdentifierPart(fileContent[cursor]));
        const span<const u8> identifier = span<const u8>(fileContent.data() + origin, cursor - origin);
        return {.type = detail::detectKeyword(identifier), .payload = identifier};
    }
}