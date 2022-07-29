# snark-protocol-experiment

The aim of this project was to perform a remote polynomial evaluation on a server with a proof that the computation is correct. For this we used SNARK, with the library `libsnark` and `jsnark`. A more complete explanation is given in the folder `report` with my internship report.

The library have the following directory structure:

* [__jsnark-experiment__](jsnark-experiment): protocol with jsnark on a polynomial with his coefficient ciphered
* [__libsnark-experiment__](libsnark-experiment): protocol with libsnark on a clear polynomial
* [__report__](report): contain a report that explain the different methods and our work