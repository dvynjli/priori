#ifndef __COMMON__
#define __COMMON__

#include <string>
#include <iostream>
#include <stdlib.h>

#include <unordered_set>
#include <set>
#include <queue>
#include <vector>
#include <unordered_map> 
#include <map>
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Instructions.h"

// #define DEBUG
// #define NOTRA

enum operation { ADD, SUB, MUL, DIV, MOD,       // Arithemetic opertions
                EQ, NE, LE, GE, LT, GT,         // Cmp operations
                LAND, LOR, LNOT, XOR,           // Logical operations
                BAND, BOR,                      // Bit-wise operations
                USUB,                           // Unary minus
                ALOAD, ASTORE, CMPXCHG,         // Atomic opreations
                LOAD, STORE,                    // Non-atomic load and store
                RMWADD, RMWSUB                  // Atomic RMW operations
};

enum DomainTypes {
  interval, octagon
};
using namespace std;

#endif