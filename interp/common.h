#include <string>
#include <iostream>
#include <stdlib.h>
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
//#include "ap_abstract1.h"
#include "ap_manager.h"
#include "box.h"
#include "ap_global1.h"

#define DEBUG 1

using namespace llvm;
using namespace std;

void print_msg(int count, ...){
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++){
        errs() << va_arg(args, char*);
    }
    va_end(args);
    errs() << "\n";
}

void print_debug_msg(int count, ...){
    if (DEBUG) {
        va_list args;
        va_start(args, count);
        errs() << "DEBUG: ";
        for (int i = 0; i < count; i++){
            errs() << va_arg(args, char*);
        }
        va_end(args);
        errs() << "\n";
    }
}
