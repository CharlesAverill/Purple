SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

PURPLE_EXECUTABLE="$SCRIPT_DIR/bin/purple"

${SCRIPT_DIR}/compile.sh
if [ $? -ne 0 ]; then
    exit 1
fi

if [[ ! -d "$SCRIPT_DIR/bin" ]] || [[ ! -f "$PURPLE_EXECUTABLE" ]]; then
    echo "$PURPLE_EXECUTABLE does not exist."
    exit 1
fi

${PURPLE_EXECUTABLE} "$@"
RC=$?
if [[ $RC -eq 139 || $RC -eq 134 ]]; then 
    CORE_DUMP="/var/lib/apport/coredump/$(cat /var/log/apport.log | tail -n1 | grep -o -- "core\.[^ ]*")"
    read -p "Segfault occurred, would you like to load $CORE_DUMP in GDB? [Y/n]" yn
    case $yn in 
        Y ) gdb $PURPLE_EXECUTABLE $CORE_DUMP;;
        y ) gdb $PURPLE_EXECUTABLE $CORE_DUMP;;
        * ) echo "Exiting";;
    esac
elif [ $RC -ne 0 ]; then
    exit 1
fi

./a.out
