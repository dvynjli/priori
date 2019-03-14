# abs_interp
Python based bstract Interpreter for pthread programs. Creates a LLVM pass to verify properties of LLVM IR.

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
opt -load build/interp/VerifierPass.so -verifier tests/test1_O1.ll
```

# Compile a source file
Compile with -O1
```
clang -S -emit-llvm -std=c++11 -O1 -o test1_O1.ll test1.cc
```

