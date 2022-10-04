clang-format -i $(find src include examples -name '*.[c|prp]' -o -name '*.h')

# cmake --target clean
cmake -B build

cmake --build build $@
