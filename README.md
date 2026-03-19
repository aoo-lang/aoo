# AOO

A programming language for Pallas' Cats (manuls).

A language designed for simplicity yet powerful usage, by combining C++'s power with Rust's (part of) safety and simplicity.

Also a semester "homework" for *Principles of Compiler* to *"impress"* my teacher. :)

## Properties

1. Compiled.
2. Statically typed.
3. Simple. AOO has less keywords than C89.
4. "Better" OOP by: No constructor; No inheritance; No virtual methods; Independent Interface definition.
5. Modules, not header files.
6. No any memory safety attempts because the true safety is in programmers' minds. Pointers exist and are the only way to store references.
7. Variables are constant by default.
8. Parameters are classified by mutability, not value/reference. Passing by value/reference is seamless by default and can be enforced.

## Examples

```aoo
import std:io:{println, scan};

//No `fn`, `let` keyword. Just use types.
i32 main() {
    u64! input = 0;
    scan("Number: ", input);
    //`_` is basically `auto`, `i`'s type will be deduced to `u8`. Use `..` to create ranges.
    for _ i = 0..10 {
        println("{} times {} is {}", input, i, input * i);
    }
    //No last expression return so you don't accidentally return something unexpected.
    return 0;
}
```
