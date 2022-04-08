# snark-protocol-experiment

This project give a simulation of a polynomial evaluation on a remote server with a proof of the result based on zkSNARK.
Some time computation of the evaluation are given in order to compare with some other methods. 

## Table of Contents

- [Build Guide](#build-guide)
  - [Installation](#installation)
- [Development Environment](#development-environment)
  - [Directory Structure](#directory-structure)
  - [Compilation Framework](#compilation-framework)
- [Compilation](#compilation)

## Build Guide

This repository has the following dependencies, which come from `libsnark`:

- C++ build environment
- CMake build infrastructure
- GMP for certain bit-integer arithmetic
- libprocps for reporting memory usage
- Fetched and compiled via Git submodules:
    - [libff](https://github.com/scipr-lab/libff) for finite fields and elliptic curves
    - [libfqfft](https://github.com/scipr-lab/libfqfft) for fast polynomial evaluation and interpolation in various finite domains
    - [Google Test](https://github.com/google/googletest) (GTest) for unit tests
    - [ate-pairing](https://github.com/herumi/ate-pairing) for the BN128 elliptic curve
    - [xbyak](https://github.com/herumi/xbyak) just-in-time assembler, for the BN128 elliptic curve
    - [Subset of SUPERCOP](https://github.com/mbbarbosa/libsnark-supercop) for crypto primitives needed by ADSNARK

### Installation

* On Ubuntu 16.04 LTS:

        $ sudo apt-get install build-essential cmake git libgmp3-dev libprocps4-dev python-markdown libboost-all-dev libssl-dev pkg-config

* On Ubuntu 14.04 LTS:

        $ sudo apt-get install build-essential cmake git libgmp3-dev libprocps3-dev python-markdown libboost-all-dev libssl-dev

* On Fedora 21 through 23:

        $ sudo yum install gcc-c++ cmake make git gmp-devel procps-ng-devel python2-markdown

* On Fedora 20:

        $ sudo yum install gcc-c++ cmake make git gmp-devel procps-ng-devel python-markdown

## Development Environment

__This library includes the completed development environment. If you wish to use the provided environment, you may proceed to the [zkSNARK Application](#zksnark-application).__

### Directory Structure

The library have the following directory structure:

* [__src__](src): C++ source code
  <!-- * [__tests__](src/tests): collection of GTests -->
* [__depends__](depends): dependency libraries


Create a dependency directory, called `depends`, and add `libsnark` as a submodule.
```bash
mkdir depends && cd depends
```
```bash
git submodule add -f https://github.com/scipr-lab/libsnark.git libsnark
```

### Compilation Framework

We will use `CMake` as our compilation framework. 

Next, create a `CMakeLists.txt` file in the `depends` directory and include the `libsnark` dependency.
```cmake
add_subdirectory(libsnark)
```

## Compilation

To compile this library, start by recursively fetching the dependencies.
```bash
git submodule update --init --recursive
```
You can have an issue with the command above, if the output of the command is "The unauthenticated git protocol on port 9418 is no longer supported." you can run this command 
```bash
git config --global url."https://github.com/".insteadOf git://github.com/
```
and then you just have to run again the command to update submodule.

Note, the submodules only need to be fetched once.

Next, initialize the `build` directory.
```bash
mkdir build && cd build && cmake ..
```

Lastly, compile the library.
```bash
make
```

To run the application, use the following command from the `build` directory:
```bash
./src/main
```
