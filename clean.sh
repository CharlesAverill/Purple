function rm_if_exists() {
    [ ! -e "$1" ] || rm "$1" -r
}

rm_if_exists CMakeCache.txt
rm_if_exists CMakeFiles
rm_if_exists *.ll
rm_if_exists *.out
rm_if_exists ./bin/
rm_if_exists ./build/

mkdir bin
mkdir build
