#pragma once
#include <ostream>
#include <span>
#include <string>

namespace AOO::Lexer {
    typedef uint8_t u8;
    using std::span, std::ostream, std::string;

    enum struct TokenType : u8 {

        //Keywords
        KW_MODULE,
        KW_IMPORT,
        KW_EXPORT,
        KW_TYPE,
        KW_TRAIT,
        KW_ENUM,
    //-----------------------------
        KW_IF,
        KW_ELSE,
        KW_FOR,
        KW_BREAK,
        KW_CONTINUE,
        KW_MATCH,
        KW_RETURN,
    //-----------------------------
        KW_PUBLIC,
        KW_PRIVATE,
        KW_SELF,
        KW_OP,
    //-----------------------------
        KW_VOID,
        KW_AUTO,
        KW_AS,
        KW_VAL,
        KW_REF,
    //-----------------------------
        KW_IN,
        KW_DUP,

        //Operators
        OP_PLUS,                    // +
        OP_DOUBLE_PLUS,             // ++
        OP_PLUS_EQUAL,              // +=

        OP_DASH,                    // -
        OP_DOUBLE_DASH,             // --
        OP_DASH_EQUAL,              // -=

        OP_STAR,                    // *
        OP_STAR_EQUAL,              // *=

        OP_SLASH,                   // /
        OP_SLASH_EQUAL,             // /=

        OP_PERCENT,                 // %
        OP_PERCENT_EQUAL,           // %=

        OP_LESS,                    // <
        OP_LESS_EQUAL,              // <=
        OP_DOUBLE_LESS,             // <<
        OP_DOUBLE_LESS_EQUAL,       // <<=

        OP_GREATER,                 // >
        OP_GREATER_EQUAL,           // >=
        OP_DOUBLE_GREATER,          // >>
        OP_DOUBLE_GREATER_EQUAL,    // >>=

        OP_BAR,                     // |
        OP_DOUBLE_BAR,              // ||
        OP_BAR_EQUAL,               // |=

        OP_AMPERSAND,               // &
        OP_DOUBLE_AMPERSAND,        // &&
        OP_AMPERSAND_EQUAL,         // &=

        OP_CARET,                   // ^
        OP_CARET_EQUAL,             // ^=

        OP_TILDE,                   // ~

        OP_BANG,                    // !
        OP_DOUBLE_BANG,             // !!

        OP_EQUAL,                   // =
        OP_DOUBLE_EQUAL,            // ==
        OP_BANG_EQUAL,              // !=

        OP_DOUBLE_QUESTION,         // ??
        OP_QUESTION_COLON,          // ?:
        OP_COLON,                   // :
        OP_DOUBLE_COLON,            // ::

        OP_EQUAL_GREATER,           // =>
        OP_DASH_GREATER,            // ->
        OP_PERIOD,                  // .
        OP_DOUBLE_PERIOD,           // ..
        OP_TRIPLE_PERIOD,           // ...

        //Characters
        CH_SEMICOLON,               // ;
        CH_COMMA,                   // ,
        CH_LEFT_PAREN,              // (
        CH_RIGHT_PAREN,             // )
        CH_LEFT_BRACE,              // {
        CH_RIGHT_BRACE,             // }
        CH_LEFT_BRACKET,            // [
        CH_RIGHT_BRACKET,           // ]

        //Binary literals
        GN_BINARY_INT,
        GN_BINARY_INT_U8,
        GN_BINARY_INT_U16,
        GN_BINARY_INT_U32,
        GN_BINARY_INT_U64,
        GN_BINARY_INT_I8,
        GN_BINARY_INT_I16,
        GN_BINARY_INT_I32,
        GN_BINARY_INT_I64,

        //Octal literals
        GN_OCTAL_INT,
        GN_OCTAL_INT_U8,
        GN_OCTAL_INT_U16,
        GN_OCTAL_INT_U32,
        GN_OCTAL_INT_U64,
        GN_OCTAL_INT_I8,
        GN_OCTAL_INT_I16,
        GN_OCTAL_INT_I32,
        GN_OCTAL_INT_I64,

        //Decimal literals
        GN_DECIMAL_INT,
        GN_DECIMAL_INT_U8,
        GN_DECIMAL_INT_U16,
        GN_DECIMAL_INT_U32,
        GN_DECIMAL_INT_U64,
        GN_DECIMAL_INT_I8,
        GN_DECIMAL_INT_I16,
        GN_DECIMAL_INT_I32,
        GN_DECIMAL_INT_I64,
        GN_DECIMAL_FLOAT,
        GN_DECIMAL_FLOAT_F32,
        GN_DECIMAL_FLOAT_F64,

        //Hexadecimal literals
        GN_HEX_INT,
        GN_HEX_INT_U8,
        GN_HEX_INT_U16,
        GN_HEX_INT_U32,
        GN_HEX_INT_U64,
        GN_HEX_INT_I8,
        GN_HEX_INT_I16,
        GN_HEX_INT_I32,
        GN_HEX_INT_I64,
        GN_HEX_FLOAT,
        GN_HEX_FLOAT_F32,
        GN_HEX_FLOAT_F64,

        //Other literals
        GN_CHAR,
        GN_STRING,
        GN_IDENTIFIER,
        GN_LABEL,

        //Misc
        MISC_WHITESPACE,
        MISC_EOF,
        MISC_ERROR
    };

    [[nodiscard]] inline const char* tokenTypeToString(TokenType type) noexcept {
        using enum TokenType;
        switch (type) {
            //Keywords
            case KW_MODULE: return "KW_MODULE";
            case KW_IMPORT: return "KW_IMPORT";
            case KW_EXPORT: return "KW_EXPORT";
            case KW_TYPE: return "KW_TYPE";
            case KW_TRAIT: return "KW_TRAIT";
            case KW_ENUM: return "KW_ENUM";
            case KW_IF: return "KW_IF";
            case KW_ELSE: return "KW_ELSE";
            case KW_FOR: return "KW_FOR";
            case KW_BREAK: return "KW_BREAK";
            case KW_CONTINUE: return "KW_CONTINUE";
            case KW_MATCH: return "KW_MATCH";
            case KW_RETURN: return "KW_RETURN";
            case KW_PUBLIC: return "KW_PUBLIC";
            case KW_PRIVATE: return "KW_PRIVATE";
            case KW_SELF: return "KW_SELF";
            case KW_OP: return "KW_OP";
            case KW_VOID: return "KW_VOID";
            case KW_AUTO: return "KW_AUTO";
            case KW_AS: return "KW_AS";
            case KW_VAL: return "KW_VAL";
            case KW_REF: return "KW_REF";
            case KW_IN: return "KW_IN";
            case KW_DUP: return "KW_DUP";
            //Operators
            case OP_PLUS: return "OP_PLUS";
            case OP_DOUBLE_PLUS: return "OP_DOUBLE_PLUS";
            case OP_PLUS_EQUAL: return "OP_PLUS_EQUAL";
            case OP_DASH: return "OP_DASH";
            case OP_DOUBLE_DASH: return "OP_DOUBLE_DASH";
            case OP_DASH_EQUAL: return "OP_DASH_EQUAL";
            case OP_STAR: return "OP_STAR";
            case OP_STAR_EQUAL: return "OP_STAR_EQUAL";
            case OP_SLASH: return "OP_SLASH";
            case OP_SLASH_EQUAL: return "OP_SLASH_EQUAL";
            case OP_PERCENT: return "OP_PERCENT";
            case OP_PERCENT_EQUAL: return "OP_PERCENT_EQUAL";
            case OP_LESS: return "OP_LESS";
            case OP_LESS_EQUAL: return "OP_LESS_EQUAL";
            case OP_DOUBLE_LESS: return "OP_DOUBLE_LESS";
            case OP_DOUBLE_LESS_EQUAL: return "OP_DOUBLE_LESS_EQUAL";
            case OP_GREATER: return "OP_GREATER";
            case OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
            case OP_DOUBLE_GREATER: return "OP_DOUBLE_GREATER";
            case OP_DOUBLE_GREATER_EQUAL: return "OP_DOUBLE_GREATER_EQUAL";
            case OP_BAR: return "OP_BAR";
            case OP_DOUBLE_BAR: return "OP_DOUBLE_BAR";
            case OP_BAR_EQUAL: return "OP_BAR_EQUAL";
            case OP_AMPERSAND: return "OP_AMPERSAND";
            case OP_DOUBLE_AMPERSAND: return "OP_DOUBLE_AMPERSAND";
            case OP_AMPERSAND_EQUAL: return "OP_AMPERSAND_EQUAL";
            case OP_CARET: return "OP_CARET";
            case OP_CARET_EQUAL: return "OP_CARET_EQUAL";
            case OP_TILDE: return "OP_TILDE";
            case OP_BANG: return "OP_BANG";
            case OP_DOUBLE_BANG: return "OP_DOUBLE_BANG";
            case OP_EQUAL: return "OP_EQUAL";
            case OP_DOUBLE_EQUAL: return "OP_DOUBLE_EQUAL";
            case OP_BANG_EQUAL: return "OP_BANG_EQUAL";
            case OP_DOUBLE_QUESTION: return "OP_DOUBLE_QUESTION";
            case OP_QUESTION_COLON: return "OP_QUESTION_COLON";
            case OP_COLON: return "OP_COLON";
            case OP_DOUBLE_COLON: return "OP_DOUBLE_COLON";
            case OP_EQUAL_GREATER: return "OP_EQUAL_GREATER";
            case OP_DASH_GREATER: return "OP_DASH_GREATER";
            case OP_PERIOD: return "OP_PERIOD";
            case OP_DOUBLE_PERIOD: return "OP_DOUBLE_PERIOD";
            case OP_TRIPLE_PERIOD: return "OP_TRIPLE_PERIOD";
            //Characters
            case CH_SEMICOLON: return "CH_SEMICOLON";
            case CH_COMMA: return "CH_COMMA";
            case CH_LEFT_PAREN: return "CH_LEFT_PAREN";
            case CH_RIGHT_PAREN: return "CH_RIGHT_PAREN";
            case CH_LEFT_BRACE: return "CH_LEFT_BRACE";
            case CH_RIGHT_BRACE: return "CH_RIGHT_BRACE";
            case CH_LEFT_BRACKET: return "CH_LEFT_BRACKET";
            case CH_RIGHT_BRACKET: return "CH_RIGHT_BRACKET";
            //Binary literals
            case GN_BINARY_INT: return "GN_BINARY_INT";
            case GN_BINARY_INT_U8: return "GN_BINARY_INT_U8";
            case GN_BINARY_INT_U16: return "GN_BINARY_INT_U16";
            case GN_BINARY_INT_U32: return "GN_BINARY_INT_U32";
            case GN_BINARY_INT_U64: return "GN_BINARY_INT_U64";
            case GN_BINARY_INT_I8: return "GN_BINARY_INT_I8";
            case GN_BINARY_INT_I16: return "GN_BINARY_INT_I16";
            case GN_BINARY_INT_I32: return "GN_BINARY_INT_I32";
            case GN_BINARY_INT_I64: return "GN_BINARY_INT_I64";
            //Octal literals
            case GN_OCTAL_INT: return "GN_OCTAL_INT";
            case GN_OCTAL_INT_U8: return "GN_OCTAL_INT_U8";
            case GN_OCTAL_INT_U16: return "GN_OCTAL_INT_U16";
            case GN_OCTAL_INT_U32: return "GN_OCTAL_INT_U32";
            case GN_OCTAL_INT_U64: return "GN_OCTAL_INT_U64";
            case GN_OCTAL_INT_I8: return "GN_OCTAL_INT_I8";
            case GN_OCTAL_INT_I16: return "GN_OCTAL_INT_I16";
            case GN_OCTAL_INT_I32: return "GN_OCTAL_INT_I32";
            case GN_OCTAL_INT_I64: return "GN_OCTAL_INT_I64";
            //Decimal literals
            case GN_DECIMAL_INT: return "GN_DECIMAL_INT";
            case GN_DECIMAL_INT_U8: return "GN_DECIMAL_INT_U8";
            case GN_DECIMAL_INT_U16: return "GN_DECIMAL_INT_U16";
            case GN_DECIMAL_INT_U32: return "GN_DECIMAL_INT_U32";
            case GN_DECIMAL_INT_U64: return "GN_DECIMAL_INT_U64";
            case GN_DECIMAL_INT_I8: return "GN_DECIMAL_INT_I8";
            case GN_DECIMAL_INT_I16: return "GN_DECIMAL_INT_I16";
            case GN_DECIMAL_INT_I32: return "GN_DECIMAL_INT_I32";
            case GN_DECIMAL_INT_I64: return "GN_DECIMAL_INT_I64";
            case GN_DECIMAL_FLOAT: return "GN_DECIMAL_FLOAT";
            case GN_DECIMAL_FLOAT_F32: return "GN_DECIMAL_FLOAT_F32";
            case GN_DECIMAL_FLOAT_F64: return "GN_DECIMAL_FLOAT_F64";
            //Hexadecimal literals
            case GN_HEX_INT: return "GN_HEX_INT";
            case GN_HEX_INT_U8: return "GN_HEX_INT_U8";
            case GN_HEX_INT_U16: return "GN_HEX_INT_U16";
            case GN_HEX_INT_U32: return "GN_HEX_INT_U32";
            case GN_HEX_INT_U64: return "GN_HEX_INT_U64";
            case GN_HEX_INT_I8: return "GN_HEX_INT_I8";
            case GN_HEX_INT_I16: return "GN_HEX_INT_I16";
            case GN_HEX_INT_I32: return "GN_HEX_INT_I32";
            case GN_HEX_INT_I64: return "GN_HEX_INT_I64";
            case GN_HEX_FLOAT: return "GN_HEX_FLOAT";
            case GN_HEX_FLOAT_F32: return "GN_HEX_FLOAT_F32";
            case GN_HEX_FLOAT_F64: return "GN_HEX_FLOAT_F64";
            //Other literals
            case GN_CHAR: return "GN_CHAR";
            case GN_STRING: return "GN_STRING";
            case GN_IDENTIFIER: return "GN_IDENTIFIER";
            case GN_LABEL: return "GN_LABEL";
            //Misc
            case MISC_WHITESPACE: return "MISC_WHITESPACE";
            case MISC_EOF: return "MISC_EOF";
            case MISC_ERROR: return "MISC_ERROR";
        }
    }

    inline ostream& operator<<(ostream& os, TokenType type) noexcept {
        os << tokenTypeToString(type);
        return os;
    }

    enum struct StringType : u8 {
        NotAString,
        Normal,     Normal_Escaped,
        Byte,       Byte_Escaped,
        CStyle,     CStyle_Escaped,
        Format,     Format_Escaped,
        Raw,        Raw_Escaped
    };

    [[nodiscard]] inline const char* stringTypeToString(StringType type) {
        switch (type) {
            case StringType::NotAString: return "NotAString";
            case StringType::Normal: return "Normal";
            case StringType::Normal_Escaped: return "Normal_Escaped";
            case StringType::Byte: return "Byte";
            case StringType::Byte_Escaped: return "Byte_Escaped";
            case StringType::CStyle: return "CStyle";
            case StringType::CStyle_Escaped: return "CStyle_Escaped";
            case StringType::Format: return "Format";
            case StringType::Format_Escaped: return "Format_Escaped";
            case StringType::Raw: return "Raw";
            case StringType::Raw_Escaped: return "Raw_Escaped";
        }
    }

    inline ostream& operator<<(ostream& os, StringType type) {
        os << stringTypeToString(type);
        return os;
    }

    struct Token {
        TokenType type;
        StringType strType{StringType::NotAString};
        union {
            span<const u8> payload;
            u8 charPayload;
        };
    };

    inline ostream& operator<<(ostream& os, const Token& token) {
        using enum TokenType;

        os << "Tk: " << token.type << ", ";
        if (token.strType != StringType::NotAString) os << "StringType: " << token.strType << ", ";
        os << "\"";
        switch (token.type) {
            case GN_CHAR: os << token.charPayload; break;
            default: os << string(token.payload.begin(), token.payload.end()); break;
        }
        os << '"';
        return os;
    }
}
