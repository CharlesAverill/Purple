./compile.sh

ANSI_RED='\033[0;31m'
ANSI_GREEN='\033[0;32m'
ANSI_BOLD='\033[1m'
ANSI_RESET='\033[0m'

function strings_are_okay() {
    printf "${ANSI_RESET}"
    bin/purple $2 > /dev/null
    prog_output=$(./a.out)
    if [ $? -ne 0 ] ; then
        return 1
    fi

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
        echo $2
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
35
70
140
true
true
false
true"

type_test_output="14
c
4294967294"

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
11
55
10
9
8
7
6
5
4
3
2
1
500
0"

base_test_output="61455
9
9
668
668
61455
61455
194"

function_test_output="X"

rm a.out
echo -n "[Variable Test]: "
strings_are_okay "$variable_test_output" "examples/variable_test.prp"
if [ $? -ne 0 ] ; then
    exit 1
fi

rm a.out
echo -n "[Condition Test]: "
strings_are_okay "$condition_test_output" "examples/condition_test.prp"
if [ $? -ne 0 ] ; then
    exit 1
fi

rm a.out
echo -n "[Type Test]: "
strings_are_okay "$type_test_output" "examples/type_test.prp"
if [ $? -ne 0 ] ; then
    exit 1
fi

rm a.out
echo -n "[Loop Test]: "
strings_are_okay "$loop_test_output" "examples/loop_test.prp"
if [ $? -ne 0 ] ; then
    exit 1
fi

rm a.out
echo -n "[Base Test]: "
strings_are_okay "$base_test_output" "examples/base_test.prp"
if [ $? -ne 0 ] ; then
    exit 1
fi

rm a.out
echo -n "[Function Test]: "
strings_are_okay "$function_test_output" "examples/function_test.prp"
if [ $? -ne 0 ] ; then
    exit 1
fi