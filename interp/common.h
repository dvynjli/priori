#ifndef __COMMON__
#define __COMMON__

#include <string>
#include <iostream>
#include <stdlib.h>

#include <unordered_set>
#include <queue>
#include <vector>
#include <unordered_map> 

#define DEBUG 1

enum operation { ADD, SUB, MUL, DIV, MOD,       // Arithemetic opertions
                EQ, NE, LE, GE, LT, GT,      // Cmp operations
                LAND, LOR, LNOT, XOR,           // Logical operations
                BAND, BOR,                      // Bit-wise operations
                USUB,                           // Unary minus
                ALOAD, ASTORE, CMPXCHG, RMW,    // Atomic opreations
                LOAD, STORE                     // Non-atomic load and store
};
using namespace std;

#endif