# Wishlist

These are things I would like the Purple compiler to do:

## Inline assembly

https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm
https://llvm.org/docs/LangRef.html#module-level-inline-assembly

## Inline LLVM-IR

https://llvm.org/docs/LangRef.html#function-attributes (`optnone`)

## Memory tunneling

- Open chunk of memory to be accessed between two or more simultaneously-running programs
- Likely to be approached alongside the development of upcoming PurpleOS (better name also coming)

## Struct operator overloading

## High-level features

- Better data types
    - N-byte ints/chars, halves: https://llvm.org/docs/LangRef.html#type-system
    - Automatic BigNumber arithmetic with `([0-9]+)bit (int|float)` identifiers
- https://docs.python.org/3/reference/compound_stmts.html#with
- String, array multiplication

## Standard Library

- Math
- Collections
- Window creation
- Networking
- Argument parsing
- Terminal graphics
- Forks/processes
- Regex

## Direct stack access

- Functions with direct stack access will have to be explicitly defined, e.g. `stack int myFunction(void);` to prevent standard register allocation from occurring. Stack functions will likely be decorated with `optnone` similar to inline LLVM
- Given LLVM's lack of explicit registers, it is likely the `main` function will have to contain register setup, e.g.
```
// x86
int main(int argc, char *argv[]) {
    registers(
        EAX, EBX, ECX, EDX,
        ESI, EDI, EBP, EIP, ESP
    );
    // Maybe this could also be handled by stdlib headers:
    #include <x86_registers.h>
}

// x86_registers.h
registers(
    EAX, EBX, ECX, EDX,
    ESI, EDI, EBP, EIP, ESP
);
```
Could also just do compile-time platform checking tbh, probably overthinking this
- Stack access via `push` and `pop` keywords
    - Relies on above register naming above, or maybe this can just be a "this is not a platform-independent feature, use at your own risk" kind of thing
    ```
    stack int myFunction(void) {
        // Here, variables are initialized within registers, but they are NOT tied to them (int a can be popped into any other register at any time)
        reg(EAX) int a = 5; // Declares an int "a" in register EAX with value 5
        reg(EBX) long b = 2e50; // Declares a long b" in register EBX with value 2^50
        // More declarations and assignments...
        // Oops! No more registers, push the contents of EAX, EBX onto stack
        push a;
        push b;
        // Yay, now I can use EAX and EBX for other stuff...
        // EAX and EBX still contain the values of a and b, but now that their data is on the stack we can just overwrite these registers
        // Now I need a and b back
        pop a, EAX; // This actually just means "pop 4 bytes from the stack into EAX"
        pop b, EBX; // This actually just means "pop 8 bytes from the stack into EAX"
        // (those type sizes are obv platform-specific)
    }
    ```