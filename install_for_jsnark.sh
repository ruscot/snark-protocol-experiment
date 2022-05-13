#!/bin/bash

mv file_for_jsnark/PaillierEncryptionAddition_Gadget.java jsnark/JsnarkCircuitBuilder/src/examples/gadgets/rsa/

mv file_for_jsnark/paillier_gadgets file_for_jsnark/paillier
mv file_for_jsnark/paillier jsnark/JsnarkCircuitBuilder/src/examples/gadgets/

mv file_for_jsnark/paillier_generators file_for_jsnark/paillier
mv file_for_jsnark/paillier jsnark/JsnarkCircuitBuilder/src/examples/generators/

mv file_for_jsnark/paillier_tests file_for_jsnark/paillier
mv file_for_jsnark/paillier jsnark/JsnarkCircuitBuilder/src/examples/tests/

