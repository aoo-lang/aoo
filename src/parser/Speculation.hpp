#pragma once
#include "../lexer/tokens.hpp"
#include "State.hpp"

namespace AOO::Parser {
    using Lexer::TokenType;

    //Token-only walker. Cursor must be at OP_LESS (the opening '<').
    //On success: returns true, cursor positioned at the token following the closing '>'.
    //On failure: returns false, cursor restored.
    //
    //The walker accepts a permissive set of tokens at angle-depth 1 (top of generic args). Anything inside balanced (), [], {} is unconstrained. After consuming the matching '>', the next token must be in the commit set:
    //    ( ) { } ] . : -> ; , ! * >
    [[nodiscard]] inline bool tryParseGenericArgs(Parser& p) noexcept {
        const u64 saved = p.cursor;
        if (peekType(p) != TokenType::OP_LESS) return false;
        advance(p); //consume opening '<'
        int angleDepth   = 1;
        int parenDepth   = 0;
        int braceDepth   = 0;
        int bracketDepth = 0;
        while (!atEnd(p)) {
            const TokenType t = peekType(p);
            if (parenDepth == 0 && braceDepth == 0 && bracketDepth == 0) {
                using enum TokenType;
                switch (t) {
                    case OP_LESS:
                        angleDepth++;
                        advance(p);
                        continue;
                    case OP_GREATER: {
                        angleDepth--;
                        advance(p);
                        if (angleDepth != 0) continue;
                        switch (peekType(p)) {
                            case CH_LEFT_PAREN:
                            case CH_RIGHT_PAREN:
                            case CH_LEFT_BRACE:
                            case CH_RIGHT_BRACE:
                            case CH_RIGHT_BRACKET:
                            case OP_PERIOD:
                            case OP_COLON:
                            case OP_DOUBLE_COLON:
                            case OP_DASH_GREATER:
                            case CH_SEMICOLON:
                            case CH_COMMA:
                            case OP_BANG:
                            case OP_DOUBLE_BANG:
                            case OP_STAR:
                            case OP_GREATER:
                                return true;
                            default:
                                p.cursor = saved;
                                return false;
                        }
                    }
                    case CH_LEFT_PAREN:    parenDepth++;   advance(p); continue;
                    case CH_LEFT_BRACE:    braceDepth++;   advance(p); continue;
                    case CH_LEFT_BRACKET:  bracketDepth++; advance(p); continue;
                    case CH_RIGHT_PAREN:
                    case CH_RIGHT_BRACE:
                    case CH_RIGHT_BRACKET:
                    case CH_SEMICOLON:
                    case OP_EQUAL:
                    case OP_PLUS:
                    case OP_DOUBLE_PLUS:
                    case OP_DASH:
                    case OP_DOUBLE_DASH:
                    case OP_SLASH:
                    case OP_PERCENT:
                    case OP_DOUBLE_BAR:
                    case OP_DOUBLE_AMPERSAND:
                    case OP_DOUBLE_EQUAL:
                    case OP_BANG_EQUAL:
                    case OP_GREATER_EQUAL:
                    case OP_LESS_EQUAL:
                    case OP_DOUBLE_LESS_EQUAL:
                    case OP_DOUBLE_GREATER_EQUAL:
                    case OP_PLUS_EQUAL:
                    case OP_DASH_EQUAL:
                    case OP_STAR_EQUAL:
                    case OP_SLASH_EQUAL:
                    case OP_PERCENT_EQUAL:
                    case OP_BAR_EQUAL:
                    case OP_AMPERSAND_EQUAL:
                    case OP_CARET_EQUAL:
                    case OP_QUESTION_COLON:
                    case OP_DOUBLE_QUESTION:
                    case OP_EQUAL_GREATER:
                    case OP_DOUBLE_PERIOD:
                    case OP_TRIPLE_PERIOD:
                    case KW_IF:
                    case KW_ELSE:
                    case KW_FOR:
                    case KW_BREAK:
                    case KW_CONTINUE:
                    case KW_MATCH:
                    case KW_RETURN:
                    case KW_MODULE:
                    case KW_IMPORT:
                    case KW_EXPORT:
                    case KW_TRAIT:
                    case KW_ENUM:
                    case KW_OP:
                    case KW_AS:
                    case KW_VAL:
                    case KW_REF:
                    case KW_IN:
                    case KW_DUP:
                    case MISC_EOF:
                    case MISC_ERROR:
                        p.cursor = saved;
                        return false;
                    default:
                        //Permissive: identifiers, literals, type keywords, , : :: * & ! !! ~
                        advance(p);
                        continue;
                }
            } else {
                using enum TokenType;
                switch (t) {
                    case CH_LEFT_PAREN:    parenDepth++;   break;
                    case CH_LEFT_BRACE:    braceDepth++;   break;
                    case CH_LEFT_BRACKET:  bracketDepth++; break;
                    case CH_RIGHT_PAREN:
                        if (parenDepth) parenDepth--;
                        else { p.cursor = saved; return false; }
                        break;
                    case CH_RIGHT_BRACE:
                        if (braceDepth) braceDepth--;
                        else { p.cursor = saved; return false; }
                        break;
                    case CH_RIGHT_BRACKET:
                        if (bracketDepth) bracketDepth--;
                        else { p.cursor = saved; return false; }
                        break;
                    default: break;
                }
                advance(p);
            }
        }
        p.cursor = saved;
        return false;
    }

    //Token-only walker for generic arguments inside a known type expression. This does not use the expression commit set because a declaration name may legally follow the closing angle: `Vec<T> value`.
    [[nodiscard]] inline bool tryParseTypeGenericArgs(Parser& p) noexcept {
        const u64 saved = p.cursor;
        if (peekType(p) != TokenType::OP_LESS) return false;
        advance(p);
        int angleDepth = 1;
        int parenDepth = 0;
        int braceDepth = 0;
        int bracketDepth = 0;
        while (!atEnd(p)) {
            const TokenType t = peekType(p);
            if (parenDepth == 0 && braceDepth == 0 && bracketDepth == 0) {
                if (t == TokenType::OP_LESS) {
                    angleDepth++;
                    advance(p);
                    continue;
                }
                if (t == TokenType::OP_GREATER) {
                    angleDepth--;
                    advance(p);
                    if (angleDepth == 0) return true;
                    continue;
                }
                if (t == TokenType::CH_SEMICOLON || t == TokenType::CH_RIGHT_PAREN || t == TokenType::CH_RIGHT_BRACE || t == TokenType::CH_RIGHT_BRACKET) {
                    p.cursor = saved;
                    return false;
                }
            }
            switch (t) {
                case TokenType::CH_LEFT_PAREN:    parenDepth++;   break;
                case TokenType::CH_LEFT_BRACE:    braceDepth++;   break;
                case TokenType::CH_LEFT_BRACKET:  bracketDepth++; break;
                case TokenType::CH_RIGHT_PAREN:
                    if (parenDepth) parenDepth--;
                    else { p.cursor = saved; return false; }
                    break;
                case TokenType::CH_RIGHT_BRACE:
                    if (braceDepth) braceDepth--;
                    else { p.cursor = saved; return false; }
                    break;
                case TokenType::CH_RIGHT_BRACKET:
                    if (bracketDepth) bracketDepth--;
                    else { p.cursor = saved; return false; }
                    break;
                default: break;
            }
            advance(p);
        }
        p.cursor = saved;
        return false;
    }

    //Token-only walker for a type expression. Used at statement entry to disambiguate declarations from expression statements: if `tryParseType` succeeds and the next token is LT_IDENTIFIER, we parse as a declaration; otherwise as an expression statement.
    //
    //Grammar:
    //  Type   ::= Prefix* Atom Postfix*
    //  Prefix ::= '~'
    //  Atom   ::= Ident GenericArgs? (':' Ident GenericArgs?)*  | 'void' | 'auto'
    //  Post   ::= '*' | '!' | '!!' | '&'
    [[nodiscard]] inline bool tryParseType(Parser& p) noexcept {
        const u64 saved = p.cursor;
        //Prefixes
        while (peekType(p) == TokenType::OP_TILDE) advance(p);
        //Atom
        switch (peekType(p)) {
            case TokenType::LT_IDENTIFIER:
            case TokenType::KW_VOID:
            case TokenType::KW_AUTO:
                advance(p);
                break;
            default:
                p.cursor = saved;
                return false;
        }
        //Optional generic args + scope segments
        for (;;) {
            if (peekType(p) == TokenType::OP_LESS) {
                if (!tryParseTypeGenericArgs(p)) {
                    p.cursor = saved;
                    return false;
                }
            }
            if (peekType(p) == TokenType::OP_COLON || peekType(p) == TokenType::OP_DOUBLE_COLON) {
                advance(p);
                if (peekType(p) != TokenType::LT_IDENTIFIER) {
                    p.cursor = saved;
                    return false;
                }
                advance(p);
                continue;
            }
            break;
        }
        //Postfixes
        for (;;) {
            const TokenType t = peekType(p);
            if (t == TokenType::OP_STAR
                || t == TokenType::OP_BANG
                || t == TokenType::OP_DOUBLE_BANG
                || t == TokenType::OP_AMPERSAND) {
                advance(p);
                continue;
            }
            break;
        }
        return true;
    }
}