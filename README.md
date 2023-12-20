# PRIORI
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
opt -load build/interp/VerifierPass.so -verifier -<domain> -stop-on-fail -<PrecisionLevel> tests/test1.ll
```
Supported abstract domains
- ```interval```: interval domain
- ```octagon```: octagon domain

Precision Levels:
- ```P0```: level 0: RMW's are treated as any other store operations
- ```P1```: keep at max K=?? RMW for each therad in PO. Not implemented yet
- ```P2```: keep all RMW in PO, consistency check of RMW is limited  to checking that combining them maintains TO among RMW
- ```P3```: P2 + conssistency check makes sure that interf PO of RMW is ordered after curPO of RMW

Other supported options
- ```no-print```: No debug prints. Prints only #errors and time elapsed
- ```stop-on-fail```: Stop the analysis as soon as assertion is failed
- ```eager-pruning```: Eagerly prune infeasible interference combinations
- ```no-interf-comb```: Use analysis without interference combinations
- ```merge-on-val```: Merge PO when values are same

Sample Command
/usr/bin/opt -load build/interp/VerifierPass.so -verifier -interval -stop-on-fail -P3 -no-print -merge-on-val -no-interf-comb tests/litmus/test1.ll

# Generate LLVM IR of a source file
Use -O1
```
clang -S -emit-llvm -std=c++11 -O1 test1.cc
```

