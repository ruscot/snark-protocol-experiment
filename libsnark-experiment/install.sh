#!/bin/bash
mkdir depends && cd depends
git submodule add -f https://github.com/scipr-lab/libsnark.git libsnark
echo "add_subdirectory(libsnark)" > CMakeLists.txt
cd .. && git submodule update --init --recursive

./change_libsnark.sh

mkdir build && cd build && cmake ..

make -j8