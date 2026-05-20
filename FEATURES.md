# F-0001

Single colon `:` is used as static scope traverser.

"Static scope" refers to any uninstantiatable object within a field, or compiler provided constants.

## Phase

Parsing

## Tests

`tests/features/f-0001.aoo`

# F-0002

`module` keyword is used to declare a module. Module declarations can only appear as the first tokens of a file with the `module` keyword followed by a module identifier or a module path.

A file can have no module declaration, in which case it can only be used as the main entry point of an executable.

## Phase

Parsing

## Tests

`tests/features/f-0002-1.aoo`

`tests/features/f-0002-2.aoo`

`tests/features/f-0002-3.aoo`

# F-0003

The numeric literal digit separator is the single quote `'`. Underscore is not accepted as a separator and must be rejected at lex time.

```aoo
u32 a = 1'000'000; //OK
u32 b = 1_000_000; //Lexer error.
```

## Phase

Lexing

## Tests

`tests/features/f-0003.aoo`

# F-0004

Raw string fences can be escalated with `#`. A raw string opens as `r` + N×`#` + `"`, and closes only on `"` + N×`#` (exact match). The lexer must count the opening hash width and refuse to terminate on a narrower close.

```aoo
r"plain raw"
r#"contains "quotes""#
r##"contains "# fragment"##
```

## Phase

Lexing

## Tests

`tests/features/f-0004.aoo`

# F-0005

Multi-line raw strings record the file's actual newline byte sequence (`\r\n` on CRLF files, `\n` on LF) into the string data. The lexer must not normalize newlines inside `r"..."`. This is observable in the resulting `[u8]`'s length.

## Phase

Lexing

## Tests

`tests/features/f-0005.aoo`

# F-0006

`c"..."` literals reject an embedded `\0` at lex time. They auto-append one trailing `\0` and produce `u8*`. Combination with the `f` flag is forbidden.

## Phase

Lexing

## Tests

`tests/features/f-0006.aoo`

# F-0007

`match` always produces an expression. the closing `}` must be followed by `;`.

## Phase

Parsing

## Tests

`tests/features/f-0007.aoo`

# F-0008

A single match arm can match multiple constructors by combining them with `|`. The token is the bitwise-or `|`, not logical `||`, to save typing. All listed alternatives route to the same arm body; there is no implicit fall-through.

## Phase

Parsing

## Tests

`tests/features/f-0008.aoo`

# F-0009

In a tagged-union declaration, a variant written without parentheses is identical to one written with `(void)`. The two forms must produce equivalent AST and identical layout.

## Phase

Parsing

## Tests

`tests/features/f-0009.aoo`

# F-0010

Loop labels are introduced with the `'name:` sigil; targeting one from `break`/`continue` requires the `'`. A bare identifier after `break`/`continue` is not legal and should trigger an error telling the user "this is not a label, consider adding `'` before".

## Phase

Parsing

## Tests

`tests/features/f-0010.aoo`

# F-0011

`import path:*` imports every directly-exported name from path. `import path:**` (double star) recursively imports every exported name from `path` and **all its descendant modules**.

The recursion is unbounded and transitive; cycle detection is the importer's responsibility.

It's suggested that the lexer don't treat `**` as a single token, but rather let the parser combine two `*` tokens into a double-star import.

## Phase

Name Resolution

## Tests

`tests/features/f-0011.aoo`

# F-0012

An import statement may introduce local aliases inline with `as`. The alias is scoped to the importing module; no separate alias keyword exists.

## Phase

Name Resolution

## Tests

`tests/features/f-0012.aoo`

# F-0013

Only explicitly exported items and whole modules are visible to import. Importing a whole module from a path does not automatically re-export its contents, only saving writing the full path to the module itself, and requires the `module` keyword.

You cannot `export` a module because the whole module importing mechanism. Modules are always "exported" in a sense.

## Phase

Name Resolution

## Tests

`tests/features/f-0013.aoo`

# F-0014

More than one item can be imported from the same path in a single statement with curly braces `{}`.

None of the items in the curly braces should be `*` or `**`.

Curly braces can only appear in the last segment of an import path.

Curly braces should be recursively nestable to allow importing from submodules.

The `as` alias syntax is also allowed within the curly braces.

Single-item curly-brace imports will result in a compiler error to enforce the code style.

## Phase

Name Resolution

## Tests

`tests/features/f-0014.aoo`

# F-0015

Multiple `type T { ... }` blocks naming the same `T` are merged across the module: member functions, in-type trait impls, and operators accumulate.

Data fields, however, must all live in exactly one block. Adding a field in a second block is a compile error.

## Phase

Name Resolution

## Tests

`tests/features/f-0015.aoo`

# F-0016

Identifiers starting with an hashtag `#` are reserved for compiler-specific identifiers.

All of them are optional to implement. However, some of them are regulated by the language and compiler implementers should put identical functionalities in those identifiers.

## Phase

Parsing

## Tests

`tests/features/f-0016.aoo`

# F-0017

`type(expr)` evaluates to the static type of `expr` and is itself a type expression — usable in declaration position, generic arguments, or anywhere a type name is. The inner expression is consumed by the type checker and never emitted as runtime code.

## Phase

Parsing

## Tests

`tests/features/f-0017.aoo`

# F-0018

The `as` keyword performs explicit type conversion. Syntax: `value-expr as type-expr`. The type expression must evaluate to a concrete type.

`as` does not change mutability or nullability. Use the postfix value operators `!` / `!!` (mutability) and `?` / `??` (nullability) before or after the cast for those.

If `B` is `auto`, deduce the target type from the context of the `as` expression following the same rules as `auto` type deduction before performing the conversion.

`A as B` performs the first applicable conversion from the following list:

1. If `type(A)` and `B` are the same type, perform nothing and emit a warning about redundant cast.
2. If `type(A)` and `B` differ only in mutability and/or nullability, emit an error directing the user to `!` / `!!` / `?` / `??`.
3. If operator `type(A):op B()` exists, `A as B` resolves to `type(A):op B(A)`.
4. If both `type(A)` and `B` are primitive number types, perform a primitive numeric cast (`static_cast`).
5. If `type(A)` is a pointer type and `B` is an unsigned integer type whose size matches the target pointer size, perform a pointer-to-integer cast for the pointer's literal address.
6. If `type(A)` is an unsigned integer type whose size matches the target pointer size, and `B` is `void*?`, perform an integer-to-pointer cast and treat the unsigned integer as a literal address.
7. If both `type(A)` and `B` are pointer types with matching nullability:
   1. If `type(A)` is `void*` or `void*?`, perform a user assertion pointer cast.
   2. If `B` is `void*` or `void*?`, perform a type erasure pointer cast.

After resolution:

1. The result's mutability matches `type(A)`.
2. For pointer-to-pointer casts (rule 7), the result's nullability matches `type(A)`.
3. For integer-to-pointer casts (rule 6), the result is always nullable `void*?`.

## Phase

Parsing

## Tests

`tests/features/f-0018.aoo`

# F-0019

The `as` keyword can also be used in enums to specify the length of the tag. The syntax is `enum E as Type`. `Type` must resolve to a primitive unsigned integer type. The tag of each variant in `E` will have the same size as `Type`.

If the number of variants in the enum exceeds the maximum value of `Type`, the compiler should emit an error.

If any of the explicitly assigned variants in the enum has a discriminant value that exceeds the maximum value of `Type`, the compiler should emit an error.

If not specified, the default tag type is the smallest unsigned integer type that can fit all the variants.

## Phase

Parsing

## Tests

`tests/features/f-0019.aoo`

# F-0020

Enums' discriminant values can be explicitly assigned with `= value` after the variant name. The assigned value must be a constant expression that evaluates to an unsigned integer.

If not specified, the first variant's discriminant value is `0`, and each subsequent variant's discriminant value is one greater than the previous variant's discriminant value.

If the next variant's discriminant value exceeds the maximum value of the enum's tag type, and the compiler cannot widen the tag type anymore (due to explicit tag type or the tag type being already the largest possible), the compiler should emit an error instead of looping around the range.

If two variants in the same enum have the same discriminant value, regardless of whether it's assigned explicitly or implicitly, the compiler should emit an error.

## Phase

Parsing

## Tests

`tests/features/f-0020.aoo`

# F-0021

Variants in an enum can have an optional payload of the form `Variant(Type)`. The payload can be of any type but not `void`, in which case the compiler should emit an error to enforce omitting `(void)` for empty payloads.

## Phase

Parsing

## Tests

`tests/features/f-0021.aoo`

# F-0022

Enums can declare itself met traits with `for Trait` syntax. The trait must be a concrete trait. The syntax should go before the `as` syntax if both are present. The compiler should check if every variant in the enum satisfies the trait's requirements and emit an error if not.

If not every variant satisfies the trait's requirements, a trait implementation (either inline or out-of-line) can be written for the enum with conventional syntax (`trait Trait in Enum {...}`) to provide the missing cases. The compiler should always assume the enum satisfies the trait's requirements if present, and dispatch to that implementation instead.

## Phase

Parsing

## Tests

`tests/features/f-0022.aoo`

# F-0023

Every field and member function declared in a type has `private` visibility by default. Every field requirement and member function declared in a trait has `public` visibility by default.

The `public` keyword and `private` keyword can be used to explicitly set the visibility of fields and member functions with two syntax forms:

`public Type fieldName;` or `private Type fieldName;`. In this form, the visibility applies only to the single field or member function being declared, and has higher precedence than the second form.

```aoo
public:
    Type fieldName;
    Type fieldName2;
```

In this form, the visibility applies to all fields and member functions declared after it until the next visibility specifier or the end of the type/trait body.

## Phase

Parsing

## Tests

`tests/features/f-0023.aoo`

# F-0024

Type / trait unions and intersections are supported with `|` and `&` respectively.

## Phase

Lexing

## Tests

`tests/features/f-0024.aoo`

# F-0025

The `Drop` trait is a special built-in trait that types can implement to run custom code when their instances go out of scope. The compiler should automatically call the `drop` method of any value that implements `Drop` when it goes out of scope, including in cases of early returns, breaks, continues, and unwinding due to panics.

The `Drop` trait has a single method `void drop(~self!)`. The void contract tracker should treat every `Drop:drop` as a special case and allow it to not pass `self` to other functions that void it.

The `Drop` trait is preluded and does not require an explicit import. However, it resides in a module of `std`.

## Phase

Parsing

## Tests

`tests/features/f-0025.aoo`

# F-0026

The `dup` keyword is used to create a bitwise copy of a value. The syntax is `dup expr`, where `expr` is the expression to be duplicated. The result of `dup expr` is a new value that is a bitwise copy of the original value with the same type.

Type authors can disable `dup` for their types by writing `dup = void;` once anywhere in the type body. The compiler should emit an error if `dup` is used on a type that has disabled it.

The operator `:=` is the syntax sugar for `dup`. `:=` can be used in most concrete cases to replace `dup`.

## Phase

Parsing

## Tests

`tests/features/f-0026.aoo`