# Style Guide

Any style rule not defined here or handled by [.clang-format](./.clang-format) is free game.

## Structs

- Names are title-cased with no separators, fields are snake-cased. 
- Use `typedef struct StructName {...} StructName;` pattern.
- Doxygen brief above struct
- Inline field documentation with no spacing between description and comment rails

```c
/**
 * @brief This struct does X
 */
typedef struct ExampleStruct {
    /**This field does y*/
    int example_field;
    /**This field does z*/
    char* second_example_field;
} ExampleStruct;
```

## Enums

- Names are uppercased
- Underscores separate words
- Uppercased enum name or abbreviation as prefix, do not alternate
- Doxygen brief above enum
- No inline field documentation, names should be descriptive enough
- Ends in a comma

```c
/**
 * @brief This enum does X
 */
typedef enum {
    EE_FIRST,
    EE_SECOND,
} ExampleEnum;
```

## Constant Arrays

- Names are camel-cased
- Doxygen brief above array
- Ends in a comma

```c
/**
 * @brief This array stores X data
 */
static const char* exampleConstantArray[] = {
    "data1", "data2",
};
```

## Functions

- Names are snake-cased
- Parameters, variables are snake-cased
- Doxygen brief, param, and return (if applicable) docs above function
- Empty line between Doxygen brief and first param/return
- Return doc starts with return type then description
- No docs in headers

```c
// function.h
int example_function(int first_param, int second_param);
// function.c
/**
 * @brief Returns sum of two numbers
 * 
 * @param first_param First number to be added
 * @param second_parm Second number to be added
 * @return int Sum of first_param and second_param
 */
int llvm_binary_arithmetic(int first_param, int second_param) {
    int x = first_param;
    int y = second_param;

    return x + y;
}
```

## File headers

Follows the below format:

```c
/**
 * @file <filename>
 * @author <author name>
 * @brief <description of file>
 * @date <DD-MMM-YYYY> // e.g. 12-Sep-2022
 */
```

## File pointers

When checking if a file has been opened successfully using a `FILE*` and `fopen(3)`, explicitly check if the returned `FILE*` is `NULL` like so:

```c
FILE* file_pointer = fopen("test.txt", "r");
if (file_pointer == NULL) {
    // Throw your error or whatever here
} 
