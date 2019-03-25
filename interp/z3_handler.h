#ifndef __Z3_HANDLER
#define __Z3_HANDLER

#include "z3++.h"
#include "llvm/IR/Instructions.h"

using namespace z3;

class Z3Helper {
	context z3Context;
	solver z3Solver;

	public:
	void initZ3();
	void addMHB(llvm::Instruction *from, llvm::Instruction *to);
}

#endif