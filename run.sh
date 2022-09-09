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
