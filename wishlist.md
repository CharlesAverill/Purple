# Wishlist

These are things I would like the Purple compiler to do:

1. Direct code generation 
    - https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm
    - https://llvm.org/docs/LangRef.html#module-level-inline-assembly
2. Direct LLVM-IR generation
    - https://llvm.org/docs/LangRef.html#function-attributes (`optnone`)
3. Memory tunneling
    - Open chunk of memory to be accessed between two or more simultaneously-running programs
    - Likely to be approached alongside the development of upcoming PurpleOS (better name also coming)
4. Struct operator overloading
5. High-level features
    - Better data types
        - N-byte ints/chars, halves: https://llvm.org/docs/LangRef.html#type-system
        - Automatic BigNumber arithmetic with `([0-9]+)bit (int|float)` identifiers
    - https://docs.python.org/3/reference/compound_stmts.html#with
    - String, array multiplication
6. Standard Library
    - Math
    - Collections
    - Window creation
    - Networking
    - Argument parsing
    - Terminal graphics
    - Forks/processes
    - Regex