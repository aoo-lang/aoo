# Lexical structure

This chapter defines the source representation of an AOO program and the rules by which a source file is decomposed into tokens. The token stream produced here is the input to all subsequent chapters.

## 1.1 Source representation

A source file is a sequence of bytes. A conforming implementation must accept input that is either pure UTF-8 or UTF-8 prefixed by a single byte-order mark (the three-byte sequence `EF BB BF` at offset zero). No other encoding is recognized. A byte-order mark anywhere other than offset zero is ill-formed.

The byte-order mark, where present, is consumed and discarded; it does not produce a token.

A *line break* is one of `LF` (`U+000A`), `CR` (`U+000D`), or the two-byte sequence `CR LF`. All three forms are accepted interchangeably and are treated as equivalent by the lexer, except that within a raw string literal (§1.8) the bytes of the line break are preserved verbatim.

Outside string and character literals, only the following bytes may appear in source: ASCII letters, ASCII digits, the ASCII whitespace characters listed in §1.2, and the ASCII punctuation listed in §§1.5.1 and 1.10. Any other byte is ill-formed at the lexical level. Within a non-raw string literal, non-ASCII UTF-8 sequences are preserved verbatim and contribute their bytes to the literal.

## 1.2 Whitespace

The whitespace characters are:

- `U+0020` (space),
- `U+0009` (horizontal tab),
- `U+000A` (line feed),
- `U+000D` (carriage return).

A maximal run of whitespace forms a single *whitespace token*. Whitespace tokens are produced by the lexer and made available to the parser; they are not silently discarded. Their primary role is to disambiguate adjacent operator characters where the lexer does not combine them (see §1.10).

## 1.3 Comments

There are two comment forms:

- *Line comment.* Begins with `//` and extends to the next `LF` or to the end of the file, whichever comes first. The terminating line break is not part of the comment.
- *Block comment.* Begins with `/*` and extends to the next `*/`. Block comments do **not** nest: the first `*/` after a `/*` closes the comment, even if intervening `/*` sequences appear.

A block comment that runs off the end of the source without a closing `*/` is ill-formed.

Comments produce no tokens. They are removed before further tokenization but cannot appear inside a token.

## 1.4 Tokens

The lexer produces a sequence of *tokens*, terminated by an end-of-file token (§1.11). Each token has a *kind* and may carry a *payload* (the source span from which it was derived, or — for character literals — a resolved one-byte value).

The token kinds are:

- whitespace,
- identifier,
- compiler identifier, macro name, macro parameter, AST binding (collectively, *sigil-prefixed identifiers*; see §1.5.1),
- keyword,
- integer literal, with base and optional type suffix,
- floating-point literal, with base and optional type suffix,
- character literal,
- string literal, with prefix flags,
- label,
- operator,
- punctuator,
- end-of-file.

Tokenization is greedy: at each position the lexer consumes the longest sequence of bytes that forms a valid token, except as noted in §1.10.

## 1.5 Identifiers and keywords

An *identifier* matches the regular expression

```
[A-Za-z_][A-Za-z0-9_]*
```

Identifiers are ASCII-only. A non-ASCII byte may not appear inside an identifier even if it would form a valid Unicode letter; programs that need non-ASCII text must place it in string literals or comments.

The following identifiers are *keywords* and may not be used as user-defined names:

```
module    import   export   type      trait     enum
if        else     for      break     continue  match    return
public    private  self     op
void      auto     as       val       ref
in        dup
```

Capitalization is significant; `If` is an identifier, not a keyword.

The following identifiers are *predefined* but not reserved at the lexical level. They name the primitive numeric and boolean types and the two boolean values, and they are made visible by the compiler-provided prelude (Chapter 6). A program may shadow them like any other identifier.

```
u8 u16 u32 u64
i8 i16 i32 i64
f32 f64
bool
true false
```

*Note.* Of the type-related names, only `void` and `auto` are keywords. `u8`, `bool`, `true`, etc. are ordinary identifiers whose meaning comes from the prelude.

### 1.5.1 Sigil-prefixed identifiers

Four ASCII characters serve as identifier-prefix sigils. In each case the sigil and an identifier (in the sense above) together form a single token whose kind is determined by the sigil:

- `#name` — *compiler identifier.* Reserved for compiler-provided features. Optional per the introduction's *Reserved compiler identifiers* rule; full semantics in Chapter 9.
- `@name` — *macro name.* Used to invoke a macro defined with the same name. See Chapter 9.
- `$name` — *macro parameter.* Used inside macro definitions to refer to a captured AST node. See Chapter 9.
- `` `name `` — *AST binding.* Used to name a stored AST fragment. See Chapter 9.

The sigil and the identifier are lexed together; whitespace between the sigil and the identifier separates them and produces an ill-formed token. An implementation that does not provide the metaprogramming facilities of Chapter 9 must reject any token using one of these sigils with a diagnostic.

## 1.6 Numeric literals

A numeric literal denotes an integer or a floating-point value. The lexer classifies each numeric literal by *base* and by whether it is an *integer* or a *floating-point* literal, and emits an optional *type suffix*.

### 1.6.1 Bases

Four bases are supported:

- *Decimal.* No prefix. Begins with a decimal digit. Leading zeros are permitted; `00`, `01`, and `09` are well-formed decimal integer literals.
- *Binary.* Prefix `0b` or `0B`, followed by binary digits.
- *Octal.* Prefix `0o` or `0O`, followed by octal digits.
- *Hexadecimal.* Prefix `0x` or `0X`, followed by hex digits. Hex digits are `0–9 a–f A–F`.

A literal whose base prefix is followed by no valid digit is ill-formed.

### 1.6.2 Digit separators

The apostrophe `'` may appear between digits as a visual separator: `1'000'000`, `0xFFFF'FFFF`. The underscore `_` is **not** a digit separator (it is reserved for hexadecimal floating-point suffixes; see §1.6.4).

A `'` may not appear immediately after a base prefix, immediately before a type suffix, immediately before or after the fractional point or an exponent indicator, or adjacent to another `'`.

### 1.6.3 Floating-point form

A literal is a *floating-point literal* if and only if it contains a fractional point `.`, an exponent, or both. The exponent indicator depends on the base:

- Decimal floats use `e` or `E`, optionally followed by a sign `+` or `-`, then decimal digits.
- Hexadecimal floats use `p` or `P`, optionally followed by a sign `+` or `-`, then decimal digits. (The exponent value is decimal even though the significand is hexadecimal.)

Binary and octal floating-point literals are not supported.

The fractional point requires at least one digit on each side. `1.0` is a floating-point literal; `1.` and `.5` are not. A leading `.` is the period operator (§1.10) followed by an integer literal.

### 1.6.4 Type suffixes

A numeric literal may carry a type suffix that fixes its type at the lexical level. Suffixes are written immediately after the digits (and after any exponent), with no separator unless noted.

- *Integer suffixes*, valid for any base: `u8`, `u16`, `u32`, `u64`, `i8`, `i16`, `i32`, `i64`.
- *Decimal floating-point suffixes:* `f32`, `f64`.
- *Hexadecimal floating-point suffixes:* `_f32`, `_f64`. The leading underscore is required and serves only to disambiguate the suffix from a hexadecimal digit `f`.

The underscore `_` is reserved for the hexadecimal-float-suffix delimiter and has no other lexical role.

A floating-point literal must not carry an integer suffix; an integer literal must not carry a floating-point suffix; a binary or octal literal must not be floating-point.

A literal that carries no type suffix has its type determined at the level of expressions (Chapter 3). The rule, summarized: if the literal's value is computable at compile time and is non-negative, it is given the smallest unsigned integer type among `u8`, `u16`, `u32`, `u64` that can represent it; if its value is negative or cannot be computed at compile time, it is given the smallest signed integer type among `i8`, `i16`, `i32`, `i64` that can represent it. A literal whose value cannot fit in any of these types is ill-formed.

### 1.6.5 Examples

```
0           42          1'000'000     0u8       42i32
0b1010      0o755       0xFFu64       0xFFFF'FFFFi64
1.0         3.14e-2     2.5f32        1.0e10f64
0x1.8p4_f32 0xA.Bp-1_f64
```

## 1.7 Character literals

A *character literal* denotes a single byte of type `u8`. It is written `'X'` where `X` is one of:

- a single ASCII byte other than `'`, `\`, `LF`, or `CR`;
- an escape sequence (§1.7.1).

A character literal whose body is a multi-byte UTF-8 sequence is ill-formed; programs that need non-ASCII characters use string literals (§1.8). An empty character literal `''` is ill-formed.

### 1.7.1 Escape sequences

Inside a character literal the following escape sequences are recognized:

- *Single-character escapes:* `\a`, `\b`, `\f`, `\n`, `\r`, `\t`, `\v`, `\\`, `\'`, `\?`. Each maps to the corresponding control or punctuation byte.
- *Octal escape:* `\` followed by one, two, or three octal digits (`0–7`). The escape's value is the octal interpretation of the digit sequence; values exceeding 255 are ill-formed.
- *Hex escape:* `\x` followed by one or more hex digits. The escape ends at the first byte that is not a hex digit. Values exceeding 255 are ill-formed.
- *Unicode escape:* `\u{HHHH}` (exactly four hex digits between the braces) or `\u{HHHHHHHH}` (exactly eight hex digits between the braces). The braces are required. The resulting code-point value must fit in a `u8`; values exceeding 255 are ill-formed.

No other escape sequences are recognized inside a character literal; an unrecognized backslash escape is ill-formed.

## 1.8 String literals

A *string literal* is a sequence of bytes between double quotes `"`, optionally preceded by *prefix flags* and *raw fences*.

### 1.8.1 Prefix flags

Three single-letter flags may appear, in any order, before the opening `"`:

- `c` — *C-style.* The literal's bytes are the body bytes followed by an implicit `\0` terminator. A `\0` byte (or `\0` escape) inside the body is ill-formed. The flag `c` may not be combined with `f`.
- `r` — *raw.* Backslash escapes are not interpreted; line breaks may appear inside the body and are preserved verbatim. Raw fences (§1.8.2) require this flag.
- `f` — *formatted.* The literal participates in compile-time formatting checks. The flag `f` may not be combined with `c`.

Each flag may appear at most once. The combinations `cf` and `fc` are ill-formed; all other combinations of a subset of `{c, r, f}` (excluding the just-named conflict) are well-formed.

*Note.* The static checks performed for `f"…"` are implementation-defined; the specification fixes only the lexical syntax of the literal. An implementation that does not provide formatted-string checking must still accept the prefix and treat the literal as an ordinary string.

Any escape sequences in a non-raw string are subject to the same rules as in character literals (§1.7.1), augmented with `\"` for an embedded double quote, and the code-point values of the resulting bytes may exceed the `u8` limit of character literals.

The presence of the `f` and `c` flag does not affect the recognition of escape sequences.

### 1.8.2 Raw fences

A raw string may be fenced by `#` characters between the prefix flags and the opening `"`. The closing delimiter is `"` followed by exactly the same number of `#` characters. Examples:

```
r"hello"            // 0-fence raw string
r#"He said "hi""#   // 1-fence raw string
r##" "# "##         // 2-fence raw string
```

`#` characters between the flags and the opening `"` are permitted only when the `r` flag is present. The body of a raw string ends at the first occurrence of `"` followed by exactly the matching number of `#`s; an earlier `"` followed by fewer `#`s, or an unfenced `"`, is part of the body.

### 1.8.3 Body, escapes, and newlines

Outside a raw string, an unescaped `LF` or `CR` within the body is ill-formed. The escape sequences of §1.7.1 are recognized, augmented with `\"` for an embedded double quote.

Inside a raw string, all bytes are taken verbatim; `\` and line breaks have no special meaning, and the choice of `LF`, `CR`, or `CR LF` for an embedded line break is preserved.

The body of a non-raw string may contain non-ASCII UTF-8 sequences directly; they are preserved as-is and contribute their bytes to the literal with no interpretation.

### 1.8.4 Examples

```
"hello\n"
c"NUL-terminated"
r#"raw with "quotes" and \backslashes"#
f"x = {}"
```

## 1.9 Labels

A *label* is written `'name:` where `name` is an identifier in the sense of §1.5. The leading `'` and trailing `:` are part of the token; the token's payload is `name`.

Labels are disambiguated from character literals by lookahead. A `'` is the start of a label when it is followed by an identifier-start byte and a (possibly empty) sequence of identifier-part bytes terminated by `:`. A `'` is the start of a character literal when it is followed by exactly one body byte (an ASCII byte or a complete escape sequence) and a closing `'`. Any other configuration is ill-formed.

## 1.10 Operators and punctuators

The lexer combines the following multi-byte operator forms by greedy match. When the longer form's bytes are present, the longer form is produced.

```
++   +=
--   -=   ->
*=
/=
%=
==   !=
=>
||   &&
!!
??   ?:
::
..   ...
```

The single-byte operators and punctuators are:

```
+ - * / %    | & ^ ~ !    =    .  :  ;  ,    (  )  {  }  [  ]
```

The character `?` may not appear as a standalone token; it appears only as part of `??` or `?:`.

The characters `<` and `>` are *always* lexed as single-byte tokens. The forms `<=`, `>=`, `<<`, `>>`, `<<=`, and `>>=` are recognized by the parser, not the lexer. The parser uses the absence of a whitespace token between adjacent `<` (or `>`) tokens to decide whether they belong to a single operator or to separate generic-argument delimiters.

*Note.* The form `vec<vec<u64>>` is lexed as the seven tokens `vec`, `<`, `vec`, `<`, `u64`, `>`, `>` with no whitespace between the two trailing `>`s. The parser may consume the two `>` tokens as a single closer of a generic argument list. Likewise, `<` immediately followed by `=` (no whitespace token between them) may be consumed as the comparison operator `<=`.

The `'` and `"` characters are not exposed as standalone operators; they appear only as the delimiters of character literals, labels, and string literals.

## 1.11 End of file

When the lexer has consumed all bytes of the source, it emits a single end-of-file token. Subsequent calls produce additional end-of-file tokens; the lexer is idempotent at end of input.
