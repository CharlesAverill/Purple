# Purple Language Documentation

The standard language specification and compiler for Purple, a general-purpose, C-like language intended to introduce higher-level functionality and lower-level memory access.

[Original Implementation](https://github.com/CharlesAverill/purple_archive/)


<!--

Purple is a simple compiled language. Right now it supports:
- Compilation into x86, MIPS
- Basic arithmetic parsing with precedence
- Comparisons
- Variable declaration and assignment
- If, Else statements
- While Loops

-->

## Installation

View the source <a href="https://github.com/CharlesAverill/Purple" target="_blank">here</a>.

### Dependencies

- `clang-{10+}`
- `libclang-{10+}-dev`

Purple may be built with cmake:
```bash
cmake -B build
cmake --build build

bin/purple example_file.prp
```

## Grammar

BNF-formatted grammar for Purple can be found here: [Purple Grammar Documentation](purple.g)

## Examples

Purple uses C-style syntax, although this may change as the compiler is built

```c
void main(void) {
    int bob;
    int alice;

    bob = 5;
    alice = 10;

    print bob + alice; // 15
    print bob > alice; // 0

    int bob_alice;
    bob_alice = 10 + 10 + bob + alice; // 35
    if(bob_alice > 30){
        print bob_alice;
    }

    int i;
    i = 0;
    while (i < 20) {
        print i;
        i = i + 1;
    } else {
        print i == 0 or true; // true
    }
}
```

More examples can be viewed <a href="https://github.com/CharlesAverill/Purple/tree/main/examples" target="_blank">here</a>.