#ifndef _PO
#define _PO

#include "common.h"
#include <sstream>

class PartialOrder {
	// all the instructions ordered after a in partial order are given 
	// by order[a]. In other words, Va.Vb (a,b) \in po => (b in order[a]).
	// If an instruction inst is not ordered with respect to any 
	// instruction in partial order so far, order[inst] will be empty 
	// set and ~E x. inst \in order[x]
	map<llvm::Instruction*, set<llvm::Instruction*>> order;

	// update ordering relation to get transitivity
	bool makeTransitiveOrdering(llvm::Instruction* from, 
		llvm::Instruction* to, 
		std::map<llvm::Instruction *, std::set<llvm::Instruction *>>::iterator toItr);

public:
	// Adds (from, to) to order if not already. 
	// Since partial order can't be cyclic, if (to, from) are already
	// in order, returns false. Else add (from, to) and returns true
	bool addOrder(llvm::Instruction* from, llvm::Instruction* to);

	// Adds inst such that Va \in order, (a, inst) \in order.
	// Returns false if inst \in order and order[inst] != {}
	bool append(llvm::Instruction* inst);

	// Joins two partial orders maintaing the ordering relation in both
	// If this is not possible (i.e. joining will result in cycle), 
	// returns false
	bool join(PartialOrder other);

	// checks if (inst1, inst2) \in order
	bool isOrderedBefore(llvm::Instruction* inst1, llvm::Instruction* inst2);

	// checks if inst is a part of this partial order
	bool isExists(llvm::Instruction* inst);

	// Removes inst from the element of the set. It should also remove
	// all (x, inst) and (inst, x) pair from order for all possible 
	// values of x
	bool remove(llvm::Instruction* inst);

	string toString();
};

#endif