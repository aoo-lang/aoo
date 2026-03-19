# AOO

A programming language for Pallas' Cats (manuls).

A language designed for simplicity yet powerful usage, by combining C++'s power with Rust's (part of) safety and simplicity.

Also a semester "homework" for *Principles of Compiler* to *"impress"* my teacher. :)

## Properties

1. Compiled.
2. Statically typed.
3. Simple. AOO has less keywords than C89.
4. "Better" OOP by: No constructor; No inheritance; No virtual methods; Independent interface definition.
5. Modules, not header files.
6. No any memory safety attempts because the true safety is in programmers' minds.
7. Variables are constant by default. Append `!` to type name to create mutable variables. 
8. Parameters are classified by mutability, not value/reference. Immutable parameters is automatically determined to be passed by value/reference but can also be specified. Mutable parameters are always passed by reference.
9. `=` always moves the right-hand side and changes its type to `?`(`void`) to prevent further usage. Implement `Copy` trait for shallow copy, and `Clone` trait for deep clone.
10. Pointer types are not hidden and it's the only way to explicitly reference to another object. No more reference rabbit holes. And also, `?*`(`void*`) exists.

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
