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
opt -load build/interp/VerifierPass.so -verifier thread_test.ll
```