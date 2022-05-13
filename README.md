# snark-protocol-experiment

The aim of this project is too give a simulation of a polynomial evaluation on a remote server with a proof of the result based on zkSNARK and FHE (fully homomorphic encryption). And compare different methods.

The library have the following directory structure:

* [__libsnark-experiment__](libsnark-experiment): protocol with libsnark on a clear polynomial
* [__report__](report): contain a report that explain the different methods
* [__homomorphic-authentication-fhe__](homomorphic-authentication-fhe): todo


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

- Run 
```bash
git clone --recursive https://github.com/akosba/jsnark.git
```

- Run:

```bash
cd jsnark/
```

Download BouncyCastle:

```bash
wget https://www.bouncycastle.org/download/bcprov-jdk15on-159.jar
```
	
- Run:

```bash
cd libsnark
```

```bash
git submodule init && git submodule update
```

```bash
mkdir build && cd build && cmake ..
```

```bash
make
```

From the root of the git repository run 
```bash
./install_for_jsnark.sh
```

### Running and Testing JsnarkCircuitBuilder
To compile the JsnarkCircuitBuilder project via command line, from the jsnark directory:

```bash
cd JsnarkCircuitBuilder
```
```bash
mkdir -p bin
```
```bash
javac -d bin -cp /usr/share/java/junit4.jar:../bcprov-jdk15on-159.jar  $(find ./src/* | grep ".java$")
```
Command to run paillier's tests
```bash
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier
```
Command to run pailliers' generator :
```bash
java -cp bin examples.generators.paillier.HornerPolynomialPaillierGenerator <number_of_coefficients_for_polynomials>
```
