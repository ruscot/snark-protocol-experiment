#!/bin/bash

echo "Add the repository jsnark"
git clone --recursive https://github.com/akosba/jsnark.git
cd jsnark/ && wget https://www.bouncycastle.org/download/bcprov-jdk15on-159.jar

echo "Compilation of libsnark in jsnark"
cd libsnark && git submodule init && git submodule update
mkdir build && cd build && cmake .. 
make -j8

echo "Install our script in jsnark"
cd ../../.. && ./install_file_for_jsnark.sh
cd jsnark/libsnark/build/ && cmake ..
make && cd ../../..	

echo "Compilation step for jsnark script"
cd jsnark/JsnarkCircuitBuilder
mkdir -p bin
javac -d bin -cp /usr/share/java/junit4.jar:../bcprov-jdk15on-159.jar  $(find ./src/* | grep ".java$")
