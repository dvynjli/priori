# abs_interp
C++ based Abstract Interpreter for C++11 programs. An LLVM pass that verifies properties of LLVM IR.

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
opt -load build/interp/VerifierPass.so -verifier tests/test1.ll
```

# Compile a source file
Compile with -O1
```
clang -S -emit-llvm -std=c++11 -O1 -o test1_O1.ll test1.cc
```

