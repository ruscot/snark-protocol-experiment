#!/bin/bash
poly_coef=( 256 512 1024 2048 4096 8192 16384 32768 65536 131072 )

cd build 
make -j16

for i in "${poly_coef[@]}"
do
	echo "$i"
    ./src/snark_polynomial_in_clear $i > output$i
done
