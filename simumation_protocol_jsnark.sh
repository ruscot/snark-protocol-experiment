#!/bin/bash
if [ "1" -gt "$#" ]; then
	echo "You have to give a number as entry for the number of coefficients"
	echo "Be careful the memory size needed for the computation is huge, start with 2 or 3 coefficients"
else 
	echo "Run setup for client with $1 polynomials"
	cd jsnark/JsnarkCircuitBuilder/
	java -cp bin examples.generators.paillier.HornerPolynomialPaillierGenerator $1
	echo "Run eval for server"
	java -cp bin examples.generators.paillier.HornerPolynomialPaillierFromFileGenerator
	cd ..
	echo "Run libsnark protocol"
	cd libsnark/build
	./libsnark/jsnark_interface/run_ppzksnark_paillier ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadgetWithoutEval.arith ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadgetWithoutEval.in ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadget_input_file.arith ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadget_input_file.in
	cd ../..
	echo "Size of file for server"
	cd JsnarkCircuitBuilder/
	wc -c < AllPailliersCoeffAndInformation.out 
	cd ../..
fi

