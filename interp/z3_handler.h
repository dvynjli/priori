#ifndef __Z3_HANDLER
#define __Z3_HANDLER

#include "common.h"

#include "z3++.h"
#include "llvm/IR/Instructions.h"

class Z3Helper {
	z3::context zcontext;
	z3::solver zsolver;
	z3::fixedpoint zfp;


	void enum_sort_example();

	public:
	Z3Helper() : zsolver(z3::solver(zcontext)), zfp(z3::fixedpoint(zcontext)) {}
	void initZ3(vector<string> globalVars);
	void addMHB(llvm::Instruction *from, llvm::Instruction *to);
};

#endif