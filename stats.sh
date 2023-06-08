function help() {
    echo "Purple project statistics"
    echo "-------------------------"
    echo "USAGE:    stats.sh [OPTIONS]"
    echo "OPTIONS:"
    echo "  -h          Show this help message"
    echo "  -l          Count the lines of code in src/ and include/"
}

function loc() {
    wc -l $(find src -name "*.c") $(find include -name "*.h")
}

while getopts ":hl" option; do
    case $option in
        l ) 
            loc 
            exit;;
        h ) 
            help 
            exit;;
        * ) 
            echo "Unrecognized argument '$option'"
            exit;;
    esac
done

help()
