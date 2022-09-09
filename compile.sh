clang-format -i src/* include/*

# cmake --target clean
cmake -B build

cmake --build build
