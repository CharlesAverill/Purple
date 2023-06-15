if [[ $1 != "--no-compile" ]] ; then
    ./compile.sh
    if [ $? -ne 0 ] ; then
        exit 1
    fi
fi

ANSI_RED='\033[0;31m'
ANSI_GREEN='\033[0;32m'
ANSI_BOLD='\033[1m'
ANSI_RESET='\033[0m'

function strings_are_okay() {
    printf "${ANSI_RESET}"
    bin/purple $2 -O$3 --fprint-func-annotations > /dev/null
    prog_output=$(./a.out)
    prog_rc=$?
    if [ $prog_rc -ne 0 ] ; then
        printf "Exited with code $ANSI_RED$ANSI_BOLD$prog_rc\n"
        return 1
    fi

    if [ "$1" == "$prog_output" ] ; then
        printf "${ANSI_GREEN}${ANSI_BOLD}OK${ANSI_RESET}\n"
        return 0
    else
        printf "${ANSI_RED}${ANSI_BOLD}NOT OK${ANSI_RESET}\n"
        printf "${ANSI_BOLD}EXPECTED${ANSI_GREEN}\n"
        echo "<<<<<"
        echo -e $1
        echo "<<<<<"
        printf "${ANSI_RESET}${ANSI_BOLD}BUT GOT${ANSI_RED}\n"
        echo ">>>>>"
        echo -e $prog_output
        printf ">>>>>${ANSI_RESET}\n"
        echo $2
        return 1
    fi
}

# Print column headers
printf "%-25s%2s %2s\n" "Test Name" "O0" "O1"
echo "------------------------------"
function run_test() {
    printf "%-25s" "[$1]" 
    for OPTLEVEL in 0 1
    do
        [ -f a.out ] && rm a.out
        TEST_OUTPUT=$(strings_are_okay "$2" "$3" "$OPTLEVEL")
        if [ $? -ne 0 ] ; then 
            printf "%s " "$TEST_OUTPUT"
            exit 1
        else
            printf "%s " "$TEST_OUTPUT"
        fi
    done
    echo ""
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
99
99
4294967294
200
4294967507"

variable_test_output="15
35
97
98
99
100
101"

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
0
55"

base_test_output="61455
9
9
668
668
61455
61455
194"

function_test_output="89
33
42
10
6
120"

comparison_test_output="1
true
1
true
0
false
0
false"

pointer_test_output="18
18
12
12
5"

pointer2_test_output="3
3
3
5
50"

run_test    "Variable"      "$variable_test_output"     "examples/variable_test.prp"
run_test    "Condition"     "$condition_test_output"    "examples/condition_test.prp"
run_test    "Type"          "$type_test_output"         "examples/type_test.prp"
run_test    "Loop"          "$loop_test_output"         "examples/loop_test.prp"
run_test    "Base"          "$base_test_output"         "examples/base_test.prp"
run_test    "Function"      "$function_test_output"     "examples/function_test.prp"
run_test    "Comparison"    "$comparison_test_output"   "examples/comparison_test.prp"
run_test    "Empty Program" ""                          "examples/empty_prog.prp"
run_test    "Pointer"       "$pointer_test_output"      "examples/pointer_test.prp"
run_test    "Pointer 2"     "$pointer2_test_output"     "examples/pointer_test_2.prp"

rm a.ll
rm a.out
