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

static map<llvm::Instruction*, pair<unsigned short int, unsigned int>> instToNum;
static map<pair<unsigned short int, unsigned int>, llvm::Instruction*> numToInst;

inline bool isSeqBefore(llvm::Instruction* inst1, llvm::Instruction* inst2){
	// printValue(inst1); errs() << " ----sb---> "; printValue(inst2);
	if (!inst1 || !inst2) return false;
	
	auto instNum1 = instToNum.find(inst1);
	auto instNum2 = instToNum.find(inst2);
	if (instNum1 == instToNum.end() || instNum2 == instToNum.end()) {
		// errs() << "ERROR: Instruction not found in instToNum map\n";
		fprintf(stderr, "ERROR: Instruction not found in instToNum map\n");
		exit(0);
	}
	// errs() << "inst1: (" << instNum1->first << "," << instNum1->second 
	if (instNum1->second.first == instNum2->second.first && instNum1->second.second < instNum2->second.second) { 
		return true;
	}
	else {
		return false;
	}
}


#endif