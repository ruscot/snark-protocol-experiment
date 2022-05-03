## jsnark

This is a Java library for building circuits for preprocessing zk-SNARKs. The library uses libsnark as a backend (https://github.com/scipr-lab/libsnark), and can integrate circuits produced by the Pinocchio compiler (https://vc.codeplex.com/SourceControl/latest) when needed by the programmer. The code consists of two main parts:
- `JsnarkCircuitBuilder`: A Java project that has a Gadget library for building/augmenting circuits. (Check the `src/examples` package)
- `libsnark/jsnark_interface`: A C++ interface to libsnark which accepts circuits produced by either the circuit builder or by Pinocchio's compiler directly.

__Updates__: 
- The jsnark library now has several cryptographic gadgets used in earlier work ([Hawk](https://eprint.iacr.org/2015/675.pdf) and [C0C0](https://eprint.iacr.org/2015/1093.pdf)). Some of the gadgets like RSA and AES were improved by techniques from xJsnark. The gadgets can be found in [src/examples/gadgets](https://github.com/akosba/jsnark/tree/master/JsnarkCircuitBuilder/src/examples/gadgets).
- xJsnark, a high-level programming framework for zk-SNARKs is available [here](https://github.com/akosba/xjsnark). xJsnark uses an enhanced version of jsnark in its back end, and aims at reducing the background/effort required by low-level libraries, while generating efficient circuits from the high-level code. Sample examples can be found in this [page](https://github.com/akosba/xjsnark#examples-included).

### Prerequisites

- Libsnark prerequisites
- JDK 8 (Higher versions are also expected to work. We've only tested with JDKs 8 and 12.)
- Junit 4
- BouncyCastle library

For Ubuntu 14.04, the following can be done to install the above:

- To install libsnark prerequisites: 

	`$ sudo apt-get install build-essential cmake git libgmp3-dev libprocps3-dev python-markdown libboost-all-dev libssl-dev`

Note: Don't clone libsnark from `https://github.com/scipr-lab/libsnark`. Make sure to use the modified libsnark submodule within the jsnark cloned repo in the next section.

- To install JDK 8: 

	`$ sudo add-apt-repository ppa:webupd8team/java`

	`$ sudo apt-get update`

	`$ sudo apt-get install oracle-java8-installer`

Verify the installed version by `java -version`. In case it is not 1.8 or later, try `$ sudo update-java-alternatives -s java-8-oracle`

- To install Junit4: 

	`$ sudo apt-get install junit4`
	
- To download BouncyCastle:

	`$ wget https://www.bouncycastle.org/download/bcprov-jdk15on-159.jar`

### Compilation for paillier's tests
For compilation :
```bash
javac -d bin -cp /usr/share/java/junit4.jar:../bcprov-jdk15on-159.jar $(find ./src/* | grep ".java$")
```
To run paillier's tests :
```bash
java -cp bin:/usr/share/java/junit4.jar org.junit.runner.JUnitCore  examples.tests.paillier.test_paillier
```
### Author
This code is developed and maintained by Ahmed Kosba <akosba@cs.umd.edu> except for the paillier's primitive. Please email for any questions.

 
