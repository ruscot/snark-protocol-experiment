#!/bin/bash

mv file_for_jsnark/PaillierEncryptionAddition_Gadget.java jsnark/JsnarkCircuitBuilder/src/examples/gadgets/rsa/

mv file_for_jsnark/paillier_gadgets file_for_jsnark/paillier
mv file_for_jsnark/paillier jsnark/JsnarkCircuitBuilder/src/examples/gadgets/

mv file_for_jsnark/paillier_generators file_for_jsnark/paillier
mv file_for_jsnark/paillier jsnark/JsnarkCircuitBuilder/src/examples/generators/

mv file_for_jsnark/paillier_tests file_for_jsnark/paillier
mv file_for_jsnark/paillier jsnark/JsnarkCircuitBuilder/src/examples/tests/

mv file_for_jsnark/CMakeLists.txt jsnark/libsnark/libsnark/
mv file_for_jsnark/run_ppzksnark_paillier.cpp jsnark/libsnark/libsnark/jsnark_interface

mv file_for_jsnark/CircuitGenerator.java jsnark/JsnarkCircuitBuilder/src/circuit/structure/
mv file_for_jsnark/CircuitEvaluator.java jsnark/JsnarkCircuitBuilder/src/circuit/eval/

rm jsnark/JsnarkCircuitBuilder/src/gadgets/math/LongIntegerModGadget.java
mv file_for_jsnark/LongIntegerModGadget.java jsnark/JsnarkCircuitBuilder/src/gadgets/math/
