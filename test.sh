ANSI_RED='\033[0;31m'
ANSI_GREEN='\033[0;32m'
ANSI_BOLD='\033[1m'
ANSI_RESET='\033[0m'

function strings_are_okay() {
    printf "${ANSI_RESET}"
    bin/purple $2 > /dev/null
    prog_output=$(./a.out)
    if [ "$1" == "$prog_output" ] ; then
        printf "${ANSI_GREEN}${ANSI_BOLD}OK${ANSI_RESET}\n"
        return 0
    else
        printf "${ANSI_RED}${ANSI_BOLD}NOT OK${ANSI_RESET}\n"
        printf "${ANSI_BOLD}EXPECTED${ANSI_RESET}\n"
        echo "<<<<<"
        echo -e $1
        echo "<<<<<"
        printf "${ANSI_BOLD}BUT GOT${ANSI_RESET}\n"
        echo ">>>>>"
        echo -e $prog_output
        echo ">>>>>"
        return 1
    fi
}

condition_test_output="true
true
true
true
true
false
1
2
3
4
6
true"

print_test_output="14
4"

variable_test_output="15
35"

loop_test_output="0
1
2
3
4
5
6
7
8
9
10
55"

echo -n "[Condition Test]: "
strings_are_okay "$condition_test_output" "examples/condition_test.prp"

echo -n "[Arithmetic Test]: "
strings_are_okay "$print_test_output" "examples/print_test.prp"

echo -n "[Variable Test]: "
strings_are_okay "$variable_test_output" "examples/variable_test.prp"

echo -n "[Loop Test]: "
strings_are_okay "$loop_test_output" "examples/loop_test.prp"