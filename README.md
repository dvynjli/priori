# abs_interp
C++ based Abstract Interpreter for C++11 programs. An LLVM pass that verifies properties of LLVM IR.

# Dependencies
- Apron 0.9.10: Library for abstarct domain
- Z3 : SAT solver. Required to prune out infeasible interferences


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
opt -load build/interp/VerifierPass.so -verifier -<domain> -z3 tests/test1.ll
```
Supported abstract domains
- ```interval```: interval domain
- ```octagon```: octagon domain
Other supported options
- ```z3```: use z3 to prune out infeasible interferences
- ```no-print```: Do not print anything. Prints only #errors and time elapsed


# Compile a source file
Compile with -O1
```
clang -S -emit-llvm -std=c++11 -O1 test1.cc
```

