clang-format -i $(find src include)

# cmake --target clean
cmake -B build

cmake --build build
