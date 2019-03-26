#ifndef __Z3_HANDLER
#define __Z3_HANDLER

#include "common.h"

#include "z3++.h"
#include "llvm/IR/Instructions.h"

enum mem_order {RLX, ACQ, REL, ACQ_REL, SEQ_CST};

class Z3Helper {
	z3::context zcontext;
	z3::solver zsolver;
	z3::fixedpoint zfp;

	// z3::func_decl memOrder;
	// z3::func_decl vars;
	z3::func_decl isLoad;
	z3::func_decl isStore;
	z3::func_decl varOf;
	z3::func_decl memOrderOf;
	
	z3::func_decl mhb;
	z3::func_decl rf;

	z3::expr getBitVec (void *op, string name);


	public:
	Z3Helper() : 
    	zsolver (z3::solver(zcontext)),
		zfp (z3::fixedpoint(zcontext)),
    	// vars (z3::function("vars", zcontext.string_sort(), zcontext.int_sort())),
    	// functions
    	// isLoad: instr -> bool
    	isLoad (z3::function("isLoad", zcontext.bv_sort(__SIZEOF_INT__), zcontext.bool_sort())),
    	// isStore: instr -> bool
    	isStore (z3::function("isStore", zcontext.bv_sort(__SIZEOF_INT__), zcontext.bool_sort())),
    	// varOf: instr -> var
    	varOf (z3::function("varOf", zcontext.bv_sort(__SIZEOF_INT__), zcontext.int_sort())),
    	// memOrderOf: instr -> memOrder
    	memOrderOf (z3::function("memOrderOf", zcontext.bv_sort(__SIZEOF_INT__), zcontext.int_sort())),
	    // relations
    	// MHB: does a MHB b? (instr, instr) -> bool
    	mhb (z3::function("MHB", zcontext.bv_sort(__SIZEOF_INT__), zcontext.bv_sort(__SIZEOF_INT__), zcontext.bool_sort())),
    	// RF: des a RF b? (instr, instr) -> bool
    	rf (z3::function("RF", zcontext.bv_sort(__SIZEOF_INT__), zcontext.bv_sort(__SIZEOF_INT__), zcontext.bool_sort()))
		{}
	
	void initZ3(vector<string> globalVars);
	void addMHB(llvm::Instruction *from, llvm::Instruction *to);
};

#endif