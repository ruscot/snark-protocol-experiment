# jsnark-experiment

This part of the project give a simulation of a polynomial evaluation with his coefficientciphered on a remote server with a proof of the result based on zkSNARK.
Some time computation of the evaluation are given in order to compare with other protocols. 


For jsnark install and dependencies :
### Prerequisites

- Libsnark prerequisites
- JDK 8 (Higher versions are also expected to work. We've only tested with JDKs 8 and 12.)
- Junit 4
- BouncyCastle library

For Ubuntu 14.04, the following can be done to install the above:

- To install libsnark prerequisites: 

```bash
sudo apt-get install build-essential cmake git libgmp3-dev libprocps3-dev python-markdown libboost-all-dev libssl-dev
```

Note: Don't clone libsnark from `https://github.com/scipr-lab/libsnark`. Make sure to use the modified libsnark submodule within the jsnark cloned repo in the next section.

- To install JDK 8: 

```bash
sudo add-apt-repository ppa:webupd8team/java
```

```bash
sudo apt-get update
```

```bash
sudo apt-get install oracle-java8-installer
```

Verify the installed version by `java -version`. In case it is not 1.8 or later, try `sudo update-java-alternatives -s java-8-oracle`

- To install Junit4: 

```bash
sudo apt-get install junit4
```
	
### jsnark Installation Instructions
You can run the script `install_for_jsnark.sh` to do the installation or you can follow the instructions below step by step :

```bash
./install_for_jsnark.sh
```

- To do the installation yourself run the following command :
```bash
git clone --recursive https://github.com/akosba/jsnark.git
```

- Run this command to download BouncyCastle:

```bash
cd jsnark/ && wget https://www.bouncycastle.org/download/bcprov-jdk15on-159.jar
```
	
- Run:

```bash
cd libsnark && git submodule init && git submodule update
```

```bash
mkdir build && cd build && cmake .. 
```

```bash
make
```
 
```bash
cd ../../..
```

- After the install for jsnark script we need to run cmake .. and make again in libsnark :
```bash
cd jsnark/libsnark/build/ && cmake ..
```
```bash
make && cd ../../..	
```
### Running and Testing JsnarkCircuitBuilder
To compile the JsnarkCircuitBuilder project via command line, from the jsnark directory:

```bash
cd jsnark/JsnarkCircuitBuilder
```
```bash
mkdir -p bin
```
```bash
javac -d bin -cp /usr/share/java/junit4.jar:../bcprov-jdk15on-159.jar  $(find ./src/* | grep ".java$")
```
Command to run paillier's tests
``java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.<name_of_file_paillier_test> ``


For example :
```bash
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier_circuit_multiplication 
```
If you want to try libsnark on a polynomial with coefficients in paillier's, you need to run the 3 following commands in the same order.
Command to run pailliers' generator :
```bash
java -cp bin examples.generators.paillier.HornerPolynomialPaillierGenerator <number_of_coefficients_for_polynomials>
```

Command to run pailliers' server witness computation :
```bash
java -cp bin examples.generators.paillier.HornerPolynomialPaillierFromFileGenerator
```

Command to run libsnark on the generate paillier's R1CS in the folder jsnark/libsnark/build : 
```bash
./libsnark/jsnark_interface/run_ppzksnark_paillier ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadgetWithoutEval.arith ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadgetWithoutEval.in ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadget_input_file.arith ../../JsnarkCircuitBuilder/Horner_polynomial_eval_big_int_paillier_gadget_input_file.in
```
