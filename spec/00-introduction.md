# Introduction

This document is the language specification for AOO. It defines the syntax, static semantics, and runtime semantics that every conforming AOO implementation must respect.

## Scope

The specification covers:

- the lexical structure of AOO source text;
- the grammar of declarations, expressions, and statements;
- the type system, including traits, generics, and tagged unions;
- value semantics: move-by-default assignment, mutability, and the rules governing `dup`, `.clone()`, and the `Drop` trait;
- the module and import system;
- the metaprogramming facilities (compiler identifiers, AST macros);
- the names reserved in the compiler identifier namespace (`#…`) and the semantics required of any implementation that chooses to provide them.

The specification does **not** cover:

- the standard library beyond the compiler-provided prelude;
- code generation, optimization, or ABI choices, except where they are observable to a conforming program;
- tooling (build systems, package managers, formatters).

## Audience

This document is written for compiler authors, tool authors, and library authors who need precise behavior. It is not a tutorial.

## Design principles

Every normative rule in this specification is downstream of one of the following principles. They are listed here so later chapters can refer back to them rather than restate the motivation.

1. **Assignment moves.** `=` always moves the right-hand side. There are no implicit copies, including for primitive types. A copy is requested explicitly with `dup` (bitwise) or `.clone()` (deep, via the `Clone` trait).
2. **Immutable by default.** A binding is immutable unless its type is suffixed with `!`. Mutability is a property of the binding's type, not a modifier on the declaration.
3. **No hidden control flow.** There are no constructors, no destructors tied to scope exit, no overloadable assignment, no implicit conversions beyond integer widening of the same signedness and any pointer to `void*`, and no function overloading.
4. **No object-oriented inheritance.** Polymorphism is expressed at compile time through traits and at runtime through tagged unions (`enum` with payloads). There are no virtual functions and no base classes.
5. **References are not first-class.** References exist only as an invisible calling convention for function parameters. Programs that need to store indirect access use pointers.
6. **One way to initialize.** Primitive bindings are initialized with `=`; compound bindings are initialized with `{}`. Every binding must have a value at the point of declaration.
7. **No clever tricks.** Operators that mutate their operand return `void`. Statements do not produce values usable in condition position. Prefix increment and decrement do not exist. The language refuses syntax whose meaning depends on evaluation order or reader memorization.
8. **Unsafe is permitted.** Pointer reinterpretation, raw memory access, and other unsafe operations are part of the language. They are not gated behind a keyword. The language assumes the programmer is an adult.

When two readings of a rule are possible, the reading consistent with these principles is normative.

## Conformance

A *conforming implementation* is one that, given a *well-formed* AOO program, produces an executable artifact whose observable behavior matches this specification.

The specification classifies behavior into four categories:

- **Defined behavior.** The implementation must produce exactly the behavior described.
- **Implementation-defined behavior.** The implementation may choose among the alternatives the specification permits, and must document its choice. Examples: whether a parameter is passed by value or by const reference when neither `val` nor `ref` is written; the resolution scope of compiler identifiers (`#name`).
- **Unspecified behavior.** The implementation may choose freely and is not required to document the choice.
- **Undefined behavior.** The program is not well-formed at runtime; the implementation has no obligations. Reaching undefined behavior is a programmer error, not a language error. Unsafe operations are the primary source.

A program that the specification describes as ill-formed must be rejected at compile time with a diagnostic. A program that compiles but exhibits undefined behavior at runtime is not a conformance failure of the implementation.

### Reserved compiler identifiers

The specification reserves identifiers in the compiler namespace — those beginning with `#`, such as `#builtin`, `#simd`, `#if`, `#expr` — and defines the semantics each carries. Every such identifier is optional. A conforming implementation:

- **may omit** any reserved compiler identifier. Source code that references an omitted identifier is ill-formed under that implementation, and the implementation must reject it with a diagnostic.
- **must not** assign conflicting semantics to a reserved name. An implementation that exposes a reserved identifier must give it the behavior this specification defines for it, or behavior consistent with that definition.
- **may** introduce additional compiler identifiers under names this specification does not reserve, with any semantics it chooses.

This freedom applies only to the `#` namespace. The rest of the language — keywords, operators, types, traits, the move and mutability rules, the module system — is required of every conforming implementation.

## Notation

Grammar productions use a variant of EBNF:

- `A ::= B` — `A` is defined as `B`.
- `B C` — `B` followed by `C`.
- `B | C` — `B` or `C`.
- `B?` — zero or one `B`.
- `B*` — zero or more `B`.
- `B+` — one or more `B`.
- `(B C)` — grouping.
- `"keyword"` — a literal terminal.
- Italicized lowercase names are nonterminals; `monospace` names are terminals or token classes defined in the lexical chapter.

Inline code samples use AOO syntax. Samples are illustrative unless explicitly marked *normative*.

Sections marked *Note* are non-normative commentary. Sections marked *Rationale* explain why a rule exists and may be skipped.

## Document structure

The remainder of the specification is organized as follows:

1. **Lexical structure** — source encoding, tokens, identifiers, keywords, literals, comments, operators.
2. **Types** — primitive types, pointers, arrays, ranges, compound types, tagged unions, traits, type aliases.
3. **Expressions** — operators, precedence, evaluation order, the move model, `dup`, `as`, ranges, lambdas, packs.
4. **Statements** — declarations, control flow (`if`, `for`, `match`, `return`, `break`, `continue`), labels, blocks.
5. **Declarations** — bindings, functions, type definitions, trait definitions and implementations, operator overloads.
6. **Modules** — `module`, `import`, `export`, name resolution, the anonymous module rule.
7. **Generics** — type parameters, trait bounds, value parameters, compile-time evaluation.
8. **Value semantics** — move, mutability, lifetime, `Drop`, the `~` void contract.
9. **Metaprogramming** — compiler identifiers, AST node types, macros, stored AST fragments.
10. **Reserved compiler identifiers** — the names this specification reserves in the `#` namespace, the semantics required where an implementation provides them, and the rules governing additions and conflicts.