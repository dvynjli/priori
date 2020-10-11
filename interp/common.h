#ifndef __COMMON__
#define __COMMON__

#include <llvm/IR/Instruction.h>
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
#include <sstream>

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

enum PrecisionLevel {
	P0, P1, P2, P3
};

using namespace std;

class InstNum {
	unsigned short int threadid;
	unsigned short int instid;
	// llvm::Instruction* inst;

public:
	InstNum() : threadid(0), instid(0) {}
	InstNum(unsigned short int tid, unsigned short int iid) :
		threadid(tid), instid(iid) {}
	
	// is current instruction sequenced before other instruction
	bool isSeqBefore(const InstNum &other) const {
		if (threadid == other.getTid() && instid <= other.getInstid()) return true;
		else return false; 
	}

	// llvm::Instruction* getLLVMInst() {
	// 	return inst;
	// }

	unsigned short int getTid() const {
		return threadid;
	}

	unsigned short int getInstid() const {
		return instid;
	}

	string toString() const {
		std::stringstream ss;
		ss << "(" << getTid() << "," << getInstid() << ")";
		return ss.str();
	}

	bool operator== (const InstNum &other) const {
		return threadid == other.getTid() && instid == other.getInstid();
	}

	bool operator!= (const InstNum &other) const {
		return threadid != other.getTid() || instid != other.getInstid();
	}

	bool operator<  (const InstNum &other) const {
		return threadid < other.threadid || 
			(threadid==other.threadid && instid < other.getInstid());
	}
	// size_t operator() () const {
	// 	return (hash<unsigned short>()(threadid) << 8 + 
	// 		hash<unsigned short>()(instid));
	// }
};

namespace std{
template<>
struct hash<InstNum> {
	size_t operator() (const InstNum &in) const {
		return ((hash<unsigned short>()(in.getTid()) << 16) || 
			hash<unsigned short>()(in.getInstid()));
	}
};
}

// typedef pair<unsigned short int, unsigned short int> INST;

extern map<llvm::Instruction*, InstNum> instToNum;
extern map<InstNum, llvm::Instruction*> numToInst;
extern unordered_map <string, set<llvm::Instruction*>> lockVarToUnlocks;
extern unordered_set<string> lockVars;

inline llvm::Instruction* getInstByInstNum(const InstNum &instnum) {
	auto searchInst = numToInst.find(instnum);
	assert(searchInst != numToInst.end() && "ERROR: the searched instruction number does not exist in map");
	return searchInst->second;
}

inline InstNum& getInstNumByInst(llvm::Instruction* inst) {
	// fprintf(stderr, "searching %p in instToNum map\n", inst);
	// fprintf(stderr, "InstToNum map:\n");
	// for (auto it: instToNum) {
	// 	fprintf(stderr, "%p: %s\n",it.first, it.second.toString().c_str());
	// }
	// if(inst == nullptr) return InstNum(0,0);
	auto searchInstNum = instToNum.find(inst);
	assert(searchInstNum != instToNum.end() && "ERROR: the searched instruction does not exist in map");
	return searchInstNum->second;
}

inline bool isUnlockInst (llvm::Instruction *inst) {
	if (llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(inst)) {
		
        if (call->getCalledFunction()->getName().find("unlock")!=
		llvm::StringRef::npos ) { return true;}}
	return false;
}

inline bool isUnlockInst (InstNum inst) {
	llvm::Instruction *llvmInst = getInstByInstNum(inst);
	return isUnlockInst(llvmInst);
}

inline bool isLockInst (llvm::Instruction *inst) {
	if (llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(inst)) {
		
        if (call->getCalledFunction()->getName().find("lock")!=
		llvm::StringRef::npos &&
		call->getCalledFunction()->getName().find("unlock")==
		llvm::StringRef::npos ) { return true;}}
	return false;
}

inline bool isLockInst (InstNum inst) {
	llvm::Instruction *llvmInst = getInstByInstNum(inst);
	return isLockInst(llvmInst);
}

inline bool isRMWInst(const InstNum &inst) {
	if (llvm::dyn_cast<llvm::AtomicRMWInst>(getInstByInstNum(inst))) {
		// fprintf(stderr, "rmw inst\n");
		return true;
	}
	else return false;
}

// inline bool isSeqBefore(llvm::Instruction* inst1, llvm::Instruction* inst2){
// 	// fprintf(stderr, "%p --sb--> %p\n", inst1, inst2);
// 	if (!inst1 || !inst2) return false;
// 	if (inst1 == inst2) return true;
	
// 	auto instNum1 = instToNum.find(inst1);
// 	auto instNum2 = instToNum.find(inst2);
// 	assert(instNum1 != instToNum.end() && instNum2 != instToNum.end() && "ERROR: Instruction not found in instToNum map");
// 	// if (instNum1 == instToNum.end()) {
// 	// 	fprintf(stderr, "ERROR: Instruction inst1 not found in instToNum map: %p\n", inst1);
// 	// 	exit(0);
// 	// }
// 	// else if (instNum2 == instToNum.end()) {
// 	// 	fprintf(stderr, "ERROR: Instruction inst2 not found in instToNum map: %p\n", inst2);
// 	// 	exit(0);
// 	// }
// 	if (instNum1->second.first == instNum2->second.first && instNum1->second.second < instNum2->second.second) { 
// 		return true;
// 	}
// 	else {
// 		return false;
// 	}
// }

// inline bool isSeqBefore(const INST &inst1, const INST &inst2) {
// 	// if (inst1 || !inst2) return false;
// 	if (inst1 == inst2) return true;
// 	if (inst1.first == inst2.first && inst1.second < inst2.second) return true;
// 	else return false; 
// }

#endif
