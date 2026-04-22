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

    [[nodiscard]] inline TokenType keywordToTokenType(const span<const u8> kw) noexcept {
        if (equals(kw, "module")) return KW_MODULE;
        else if (equals(kw, "import")) return KW_IMPORT;
        else if (equals(kw, "export")) return KW_EXPORT;
        else if (equals(kw, "type")) return KW_TYPE;
        else if (equals(kw, "trait")) return KW_TRAIT;
        else if (equals(kw, "enum")) return KW_ENUM;
        //-----------------------------
        else if (equals(kw, "if")) return KW_IF;
        else if (equals(kw, "else")) return KW_ELSE;
        else if (equals(kw, "for")) return KW_FOR;
        else if (equals(kw, "break")) return KW_BREAK;
        else if (equals(kw, "continue")) return KW_CONTINUE;
        else if (equals(kw, "match")) return KW_MATCH;
        else if (equals(kw, "return")) return KW_RETURN;
        //-----------------------------
        else if (equals(kw, "public")) return KW_PUBLIC;
        else if (equals(kw, "private")) return KW_PRIVATE;
        else if (equals(kw, "self")) return KW_SELF;
        else if (equals(kw, "op")) return KW_OP;
        //-----------------------------
        else if (equals(kw, "void")) return KW_VOID;
        else if (equals(kw, "auto")) return KW_AUTO;
        else if (equals(kw, "as")) return KW_AS;
        else if (equals(kw, "val")) return KW_VAL;
        else if (equals(kw, "ref")) return KW_REF;
        //-----------------------------
        else if (equals(kw, "in")) return KW_IN;
        else if (equals(kw, "dup")) return KW_DUP;
        else return GN_IDENTIFIER;
    }

    // You might NOT get an identifier token from this.
    [[nodiscard]] inline Token getIdentifier(u64& cursor) noexcept {
        if (isValidIdentifierStart(fileContent[cursor])) {
            if ( //Prefixed string literals.
                (fileContent[cursor] == 'b' || fileContent[cursor] == 'c' || fileContent[cursor] == 'f' || fileContent[cursor] == 'r')
             && cursor + 1 < fileContent.size()
             && fileContent[cursor + 1] == '"'
            ) {
                cursor++;
                return getStringLiteral(cursor);
            }
            const u64 identifierStart = cursor;
            do { cursor++; } while (cursor < fileContent.size() && isValidIdentifierPart(fileContent[cursor]));
            const span<const u8> identifier = span<const u8>(fileContent.data() + identifierStart, cursor - identifierStart);
            return {.type = keywordToTokenType(identifier), .payload = identifier};
        }
        //note: UTF-8 characters are not valid identifiers, and they will go to this branch and be treated as unknown characters, but they are split up (emits more than one token from one character) in this process.
        else {
            //Unknown character
            cursor++;
            return {.type = TokenType::MISC_ERROR, .payload = span(fileContent.data() + cursor - 1, 1)};
        }
    }
}