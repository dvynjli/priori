#include "partial_order.h"


// Adds (from, to) to order if not already.  from-->to
// Since partial order can't be cyclic, if (to, from) are already
// in order, returns false. The behavior might be undefined 
// in this case. Else add (from, to) and returns true
bool PartialOrder::addOrder(llvm::Instruction* from, llvm::Instruction* to) {
	cout << "addOrder\n";
	// TODO: check if some inst sequenced before 'from' or 'to' is in
	// order. If yes, remove the older one.
	if (isOrderedBefore(from, to)) return true;
	// find 'to' in ordering
	auto toItr = order.find(to);
	if (toItr == order.end()) {
		set<llvm::Instruction*> emptyset {};
		order[to] = emptyset;
	}
	// set transitive ordering from-->to
	return makeTransitiveOrdering(from, to, toItr);
}

// Adds inst such that Va \in order, (a, inst) \in order.
// Returns false if inst already exists in order
bool PartialOrder::append(llvm::Instruction* inst) {
	// TODO: check if some inst sequenced before 'inst' in order. 
	// If yes, remove the older one.
	if (!order[inst].empty()) return false;
	for (auto it=order.begin(); it!=order.end(); ++it) {
		if (it->first != inst) {
			it->second.insert(inst);
		}
	}
	return true;
}

// Joins two partial orders maintaing the ordering relation in both
// If this is not possible (i.e. joining will result in cycle), 
// returns false
bool PartialOrder::join(PartialOrder other) {
	cout << "join";
	for (auto fromItr=other.begin(); fromItr!=other.end(); ++fromItr) {
		// fprintf(stderr, "%p ",fromItr->first);
		for (auto toItr=fromItr->second.begin(); toItr!=fromItr->second.end(); ++toItr) {
			if (!addOrder(fromItr->first, *toItr))
				return false;
		}
	}
	return true;
}

// checks if (inst1, inst2) \in order
bool PartialOrder::isOrderedBefore(llvm::Instruction* inst1, llvm::Instruction* inst2) {
	auto search = order[inst1].find(inst2);
	if (search == order[inst1].end()) return false;
	else return true;
}

// checks if inst is a part of this partial order
bool PartialOrder::isExists(llvm::Instruction* inst) {

}

// Removes inst from the element of the set. It should also remove
// all (x, inst) and (inst, x) pair from order for all possible 
// values of x
bool PartialOrder::remove(llvm::Instruction* inst) {
	for (auto it=order.begin(); it!=order.end(); ++it) {
		it->second.erase(inst);
	}
	order.erase(inst);
	return true;
}

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
		for (auto it=toItr->second.begin(); it!=toItr->second.end(); ++it) {
			order[from].insert(*it);
		}
	}

	// set transitive ordering with x s.t x-->from
	for (auto it=order.begin(); it!=order.end(); ++it) {
		auto search = it->second.find(from);
		if(search != it->second.end()) {
			if (makeTransitiveOrdering(it->first, to, toItr) == false)
				return false;
		}
	}
	return true;
}

string PartialOrder::toString() {
	std::stringstream ss; 
	for (auto itFrom: order){
		for (auto itTo: itFrom.second) {
			ss << itFrom.first; 
			ss << "---->";
			ss << itTo;
			ss << "\n";
		}
	}
	return ss.str();
}

map<llvm::Instruction*, set<llvm::Instruction*>>::iterator PartialOrder::begin() {
	return order.begin();
}

map<llvm::Instruction*, set<llvm::Instruction*>>::iterator PartialOrder::end() {
	return order.end();
}
