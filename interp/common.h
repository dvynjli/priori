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

extern map<llvm::Instruction*, pair<unsigned short int, unsigned int>> instToNum;
extern map<pair<unsigned short int, unsigned int>, llvm::Instruction*> numToInst;

inline bool isSeqBefore(llvm::Instruction* inst1, llvm::Instruction* inst2){
	// fprintf(stderr, "%p --sb--> %p\n", inst1, inst2);
	if (!inst1 || !inst2) return false;
	if (inst1 == inst2) return true;
	
	auto instNum1 = instToNum.find(inst1);
	auto instNum2 = instToNum.find(inst2);
	assert(instNum1 != instToNum.end() && instNum2 != instToNum.end() && "ERROR: Instruction not found in instToNum map");
	// if (instNum1 == instToNum.end()) {
	// 	fprintf(stderr, "ERROR: Instruction inst1 not found in instToNum map: %p\n", inst1);
	// 	exit(0);
	// }
	// else if (instNum2 == instToNum.end()) {
	// 	fprintf(stderr, "ERROR: Instruction inst2 not found in instToNum map: %p\n", inst2);
	// 	exit(0);
	// }
	if (instNum1->second.first == instNum2->second.first && instNum1->second.second < instNum2->second.second) { 
		return true;
	}
	else {
		return false;
	}
}


#endif