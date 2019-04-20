#ifndef __Z3_HANDLER
#define __Z3_HANDLER

#include "common.h"

#include "z3++.h"
#include "llvm/IR/Instructions.h"


#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Operator.h"

#define BV_SIZE (__SIZEOF_POINTER__*8)

// llvm: NA=0, RLX=2, ACQ=4, REL=5, (ACQ_REL=6), SEQ_CST=7
enum mem_order {NA, RLX, ACQ, REL, ACQ_REL, SEQ_CST};

class Z3Helper {
	z3::context zcontext;
	// z3::solver zsolver;
	z3::fixedpoint zfp;
	string rules;

	// z3::func_decl memOrder;
	// z3::func_decl vars;
	z3::func_decl isLoad;
	z3::func_decl isStore;
	z3::func_decl isVarOf;
	z3::func_decl memOrderOf;
	
	z3::func_decl mhb;
	z3::func_decl rf;
	z3::func_decl nrf;
	z3::func_decl po;
	z3::func_decl mcb;

	z3::expr getBitVec (void *op);
	z3::expr getMemOrd(llvm::AtomicOrdering ord);
	void addInstToVar(z3::expr inst, llvm::Value *var);
	void addInstToMemOrd(z3::expr inst, llvm::AtomicOrdering ord);

	void addInterference (unordered_map<llvm::Instruction*, llvm::Instruction*> interfs);
	z3::expr makeQueryOfInterference (unordered_map<llvm::Instruction*, llvm::Instruction*> interfs);
	

public:
	Z3Helper() : 
    	// zsolver (z3::solver(zcontext)),
		zfp (z3::fixedpoint(zcontext)),
    	// vars (z3::function("vars", zcontext.string_sort(), zcontext.int_sort())),
    	// functions
    	// isLoad: instr -> bool
    	isLoad (z3::function("isLoad", zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
    	// isStore: instr -> bool
    	isStore (z3::function("isStore", zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
    	// isVarOf: instr -> var
    	isVarOf (z3::function("varOf", zcontext.bv_sort(BV_SIZE), zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
    	// memOrderOf: instr -> memOrder
    	memOrderOf (z3::function("memOrderOf", zcontext.bv_sort(BV_SIZE), zcontext.int_sort(), zcontext.bool_sort())),
	    // relations
    	// MHB: does a MHB b? (instr, instr) -> bool
    	mhb (z3::function("MHB", zcontext.bv_sort(BV_SIZE), zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
    	// RF: does a RF b? (instr, instr) -> bool
    	rf (z3::function("RF", zcontext.bv_sort(BV_SIZE), zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
		// NRF: does a NRF b? (instr, instr) -> bool
    	nrf (z3::function("NRF", zcontext.bv_sort(BV_SIZE), zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
		// PO: Program Order. (instr, instr) -> bool
		po (z3::function("PO", zcontext.bv_sort(BV_SIZE), zcontext.bv_sort(BV_SIZE), zcontext.bool_sort())),
		// memoryConstraintBefore: can't reorder as per c11 memory ordering constraints
		// (instr, instr) -> bool
		mcb (z3::function("MemOrdConstraint", zcontext.bv_sort(BV_SIZE), zcontext.bv_sort(BV_SIZE), zcontext.bool_sort()))
		{
			// Z3_fixedpoint_set_params(zcontext, zfp, "datalog");
			z3::params params(zcontext);
			try {
				params.set("engine", zcontext.str_symbol("datalog"));
			} catch (z3::exception e) { cout << "Exception: " << e << endl; exit(0);}
			// params.set("datalog.default_table", zcontext.str_symbol("hashtable"));
			// params.set("datalog.magic_sets_for_queries", true);
			// cout << params << endl;
			// zfp.register_relation(isLoad);
			// zfp.register_relation(isStore);
			// zfp.register_relation(isVarOf);
			// zfp.register_relation(memOrderOf);
			// zfp.register_relation(mhb);
			// zfp.register_relation(rf);
			// zfp.register_relation(nrf);
			// zfp.register_relation(po);
			// zfp.register_relation(mcb);
		}
	
	void initZ3(vector<string> globalVars);
	void addPO(llvm::Instruction *from, llvm::Instruction *to);
	void addMHB(llvm::Instruction *from, llvm::Instruction *to);
	void addLoadInstr  (llvm::LoadInst *inst);
	void addStoreInstr (llvm::StoreInst *inst);

	bool checkInterference (unordered_map<llvm::Instruction*, llvm::Instruction*> interfs);
	string getRules();
	void addRules(string rules);
	void addMHBandPORules(vector< pair <string, pair<llvm::Instruction*, llvm::Instruction*>>> relations);
	void addAllLoads(unordered_map<llvm::Function*, unordered_map<llvm::Instruction*, string>> allLoads);
	void addAllStores(unordered_map<llvm::Function*, unordered_map<string, unordered_set<llvm::Instruction*>>> allStores);
	void addInferenceRules();

	void testFixedPoint();
	void testQuery();
};

#endif