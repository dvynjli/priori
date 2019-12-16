# abs_interp
C++ based Abstract Interpreter for C++11 programs. An LLVM pass that verifies properties of LLVM IR.

# Dependencies
- Apron 0.9.10: Library for abstarct domain
- Z3 : SAT solver. Required to prune out infeasible interferences
- gcc (Tested with 7.4.0 on Ubuntu 16.04 and 18.04)
- clang (Tested with 3.8.0 on Ubuntu 16.04 and 6.0.0 on Ubuntu 18.04)

# Building
```
    mkdir build
    cd build
    cmake ..
    make
```
Might need to run ```make``` as sudo

# Running the pass
```
opt -load build/interp/VerifierPass.so -verifier -<domain> -z3-minimal -useMOPO tests/test1.ll
```
Supported abstract domains
- ```interval```: interval domain
- ```octagon```: octagon domain

Other supported options
- ```no-print```: No debug prints. Prints only #errors and time elapsed
- ```z3-minimal```: Enable interferce pruning using Z3 using Release Head based analysis
- ```useMOHead```: Enable interference pruning using Z3 using modification order head based analysis
- ```useMOPO```: Enable interference pruning using Z3 using partial order over modification order based analysis


# Compile a source file
Compile with -O1
```
clang -S -emit-llvm -std=c++11 -O1 test1.cc
```

