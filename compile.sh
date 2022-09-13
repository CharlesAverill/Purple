clang-format -i $(find src include -name '*.c' -o -name '*.h')

# cmake --target clean
cmake -B build

cmake --build build
