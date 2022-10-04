/**
 * @file clang.h
 * @author Charles Averill
 * @brief Function headers for interacting with clang
 * @date 13-Sep-2022
 */

#ifndef CLANG_H
#define CLANG_H

/**
 * @brief Program used to determine platform information
 */
#define GENERATOR_PROGRAM_CONTENTS "#include <stdio.h>\nint main(void){printf(\"\");}"
/**
 * @brief Length of GENERATOR_PROGRAM_CONTENTS
 */
#define GENERATOR_PROGRAM_CONTENTS_LENGTH sizeof(GENERATOR_PROGRAM_CONTENTS)
/**
 * @brief Filename of GENERATOR_PROGRAM
 */
#define GENERATOR_PROGRAM_FILENAME ".prp_platform_information_generator.c"
/**
 * @brief Filename of GENERATOR_PROGRAM's LLVM-compiled file
 */
#define GENERATOR_PROGRAM_FILENAME_LL ".prp_platform_information_generator.ll"
/**
 * @brief Full path to GENERATOR_PROGRAM
 */
static char generatorProgramFullPath[512];
/**
 * @brief Full path to GENERATOR_PROGRAM's LLVM-compiled file
 */
static char generatorProgramLLFullPath[512];
/**
 * @brief True if the generator program has been written and compiled
 */
static bool generatorProgramWritten = false;
/**
 * @brief Placeholder in main LLVM file used when linking globals
 */
#define PURPLE_GLOBALS_PLACEHOLDER ";<purple_globals_placeholder>"
/**
 * @brief Length of PURPLE_GLOBALS_PLACEHOLDER
 */
#define PURPLE_GLOBALS_PLACEHOLDER_LEN sizeof(PURPLE_GLOBALS_PLACEHOLDER) - 1

void clang_compile_llvm(const char* fn);
void link_globals(void);
void create_tmp_generator_program(void);
char* get_target_datalayout(void);
char* get_target_triple(void);
char* get_postamble(void);
char* regex_match(const char* regex, char* target_str, int len, int group_index);

#endif /* CLANG_H */
