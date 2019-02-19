#ifndef __COMMON__
#define __COMMON__

#include <string>
#include <iostream>
#include <stdlib.h>

#include <set>
#include <queue>
#include <vector>

#define DEBUG 1

enum operation { ADD, SUB, MUL, DIV, MOD,      // Arithemetic opertions
                LAND, LOR, LNOT, XOR,           // Logical operations
                BAND, BOR,                      // Bit-wise Operations
                USUB,                           // Unary minus
                ALOAD, ASTORE, CMPXCHG, RMW,    // Atomic Opreations
                LOAD, STORE                     // Non-atomic load and store
};
using namespace std;

#endif