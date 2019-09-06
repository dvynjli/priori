#include "partial_order.h"

// update ordering relation to get transitivity
bool PartialOrder::makeTransitiveOrdering (llvm::Instruction* from, llvm::Instruction* to, 
	std::map<llvm::Instruction *, std::set<llvm::Instruction *>>::iterator toItr
){
	// check for anti-symmetry
	if (isOrderedBefore(to, from)) return false;

	// insert 'to' in set of instrcutions partially ordered with 'from'
	if (order[from].insert(to).second == false) {
		// 'to' was already added in the ordering. No need to update ordering furthur
		return true;
	}
	
	// set transitive ordering with 'from' of all elements x s.t. to-->x
	if (toItr != order.end()) {
		// add all the elements ordered with 'to' to 'from'
		for (auto it=toItr->second.begin(); it!=toItr->second.end; it++) {
			order[from].insert(*it);
		}
	}

	// set transitive ordering with x s.t x-->from
	for (auto it=order.begin(); it!=order.end(); it++) {
		auto search = it->second.find(from);
		if(search != it->second.end()) {
			if (makeTransitiveOrdering(it->first, to, toItr) == false)
				return false;
		}
	}
	return true;
}


// Adds (from, to) to order if not already.  from-->to
// Since partial order can't be cyclic, if (to, from) are already
// in order, returns false. The behavior might be undefined 
// in this case. Else add (from, to) and returns true
bool PartialOrder::addOrder(llvm::Instruction* from, llvm::Instruction* to) {
	// find 'to' in ordering
	auto toItr = order.find(to);
	// set transitive ordering from-->to
	return makeTransitiveOrdering(from, to, toItr);
}

// Adds inst such that Va \in order, (a, inst) \in order.
// Returns false if inst already exists in order
bool PartialOrder::append(llvm::Instruction* inst) {

}

// Joins two partial orders maintaing the ordering relation in both
// If this is not possible (i.e. joining will result in cycle), 
// returns false
bool PartialOrder::join(PartialOrder other) {

}

// checks if (inst1, inst2) \in order
bool PartialOrder::isOrderedBefore(llvm::Instruction* inst1, llvm::Instruction* inst2) {

}

// checks if inst is a part of this partial order
bool PartialOrder::isExists(llvm::Instruction* inst) {

}

// Removes inst from the element of the set. It should also remove
// all (x, inst) and (inst, x) pair from order for all possible 
// values of x
bool PartialOrder::remove(llvm::Instruction* inst) {
	
}

string PartialOrder::toString() {

}