#!/bin/bash
cd jsnark/JsnarkCircuitBuilder/
echo "Compilation"
javac -d bin -cp /usr/share/java/junit4.jar:../bcprov-jdk15on-159.jar  $(find ./src/* | grep ".java$")
echo "Test addition "
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_circuit_addition
echo "Test multiplication "
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_circuit_multiplication
echo "Test mod pow "
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_circuit_mod_pow
echo "Test mod pow big integer "
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_circuit_mod_pow_big_integer
echo "Test poly eval big integer "
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_horner_polynomial_eval_big_integer
echo "Test operation"
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_operation
cd ..
