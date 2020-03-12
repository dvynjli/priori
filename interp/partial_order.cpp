#include "partial_order.h"


// Adds (from, to) to order if not already.  from-->to
// Since partial order can't be cyclic, if (to, from) are already
// in order, returns false. The behavior might be undefined 
// in this case. Else add (from, to) and returns true
bool PartialOrder::addOrder(Z3Minimal &zHelper, llvm::Instruction* from, llvm::Instruction* to) {
	// fprintf(stderr, "addOrder %p --> %p\n", from, to);
	
	if (isOrderedBefore(from, to)) return true;
	if (isOrderedBefore(to, from)) return false;

	// find 'to' in ordering
	auto toItr = order.find(to);
	// if 'to' does not exist, add an (to,<empty>) in order 
	if (toItr == order.end()) {
		set<llvm::Instruction*> emptyset {};
		order[to] = emptyset;
	}

	// Check if some inst sequenced before 'from' or 'to' is in
	// the order. If yes, remove the older one.
	// OPT: The check is required only if 'from' or 'to' are not 
	// in the order already.
	for (auto it=order.begin(); it!=order.end(); ) {
		llvm::Instruction* inst = it->first; ++it;
		// fprintf(stderr, "Checking SB %p-->%p, %p-->%p\n", inst,to,inst,from);
		if (inst != to && inst != from) {
			if (isSeqBefore(inst, to)) {
				// fprintf(stderr, "isseqbefore inst %p to %p\n", inst, to);
				addOrder(zHelper, inst, to); 
				remove(inst);
			}
			else if (isSeqBefore(to, inst)) {
				// fprintf(stderr, "isseqbefore to %p inst %p\n",to, inst);
				return addOrder(zHelper, from, inst);
			}
			if (isSeqBefore(inst, from)) {
				// fprintf(stderr, "isseqbefore inst %p from %p\n", inst, from);
				addOrder(zHelper, inst, from); 
				remove(inst);
			}
			else if (isSeqBefore(from, inst)) {
				bool done = makeTransitiveOrdering(from, to, toItr);
				remove(from);
				// fprintf(stderr, "isseqbefore from %p inst %p\n ", from, inst);
				addInst(zHelper, to);
				return done;
			}
		}
	}

	// set transitive ordering from-->to
	return makeTransitiveOrdering(from, to, toItr);
}

// Adds inst such that Va \in order, (a, inst) \in order.
// Returns false if inst already exists in order
bool PartialOrder::append(Z3Minimal &zHelper, llvm::Instruction* newinst) {
	// cout << "appending Partial order " << newinst << "\n";
	// Check if some inst sequenced before 'inst' in order. 
	// If yes, remove the older one.
	for (auto it=order.begin(); it!=order.end(); ) {
		if (isSeqBefore(it->first, newinst)) {auto ittmp = it++; remove(ittmp->first);}
		else it++;
	}

	// If the 'newinst' is already in the order, it cannot be appended
	if (!order[newinst].empty()) return false;
	
	for (auto it=order.begin(); it!=order.end(); ++it) {
		if (it->first != newinst) {
			it->second.insert(newinst);
		}
	}

	// if(llvm::AtomicRMWInst *rmwInst = llvm::dyn_cast<llvm::AtomicRMWInst>(newinst)) {
	// 	rmws.insert(newinst);
	// }

	return true;
}

// Joins two partial orders maintaing the ordering relation in both
// If this is not possible (i.e. joining will result in cycle), 
// returns false
bool PartialOrder::join(Z3Minimal &zHelper, PartialOrder &other) {
	// fprintf(stderr, "joining\n");
	// fprintf(stderr, "%s\t and \t%s\n", toString().c_str(), other.toString().c_str());
	for (auto fromItr=other.begin(); fromItr!=other.end(); ++fromItr) {
		// fprintf(stderr, "%p ",fromItr->first);
		if (fromItr->second.empty() && order.find(fromItr->first) == order.end()){
			addInst(zHelper, fromItr->first);
		}
		for (auto toItr=fromItr->second.begin(); toItr!=fromItr->second.end(); ++toItr) {
			if (!addOrder(zHelper, fromItr->first, *toItr))
				return false;
		}
		// if (llvm::AtomicRMWInst *rmwInst = llvm::dyn_cast<llvm::AtomicRMWInst>(fromItr->first))
		// 	rmws.insert(fromItr->first);
	}
	// fprintf(stderr, "after join %s\n", toString().c_str());

	return true;
}

// checks if (inst1, inst2) \in order
bool PartialOrder::isOrderedBefore(llvm::Instruction* inst1, llvm::Instruction* inst2) {
	if (!isExists(inst1)) return false;
	auto search = order[inst1].find(inst2);
	if (search == order[inst1].end()) return false;
	else return true;
}

// checks if inst is a part of this partial order
bool PartialOrder::isExists(llvm::Instruction* inst) {
	auto search = order.find(inst);
	if (search == order.end()) return false;
	else return true;
}

// checks if the partial order other is consistent with this partial order
// Two parial orders are consistent only if  Va.Vb (a,b) \in order 
// (b,a) \notin other.order, or viceversa
bool PartialOrder::isConsistent(PartialOrder &other) {
	for (auto itFrom:other) {
		for (auto itTo:itFrom.second) {
			// if ordering is conistent 
			if (isOrderedBefore(itTo, itFrom.first)) return false;
		}
	}

	return true;
}

bool PartialOrder::isConsistentRMW(PartialOrder &other) {
	// fprintf(stderr, "checking consistent rmw:\n");
	// fprintf(stderr, "this: %s", toString().c_str());
	// fprintf(stderr, "other: %s\n", other.toString().c_str());
	// fprintf(stderr, "This rmws: ");
	// for(auto it:rmws) {
	// 	fprintf(stderr, "%p,", it);
	// }
	// fprintf(stderr, "\nOther rmws: ");
	// for(auto it:other.rmws) {
	// 	fprintf(stderr, "%p,", it);
	// }
	// fprintf(stderr, "\n");
	// for (auto itCur: rmws) {
	// 	for (auto itOther: other.rmws) {
	// 		if (!(isExists(itOther) || other.isExists(itCur))) {
	// 			// fprintf(stderr, "not consistent, isExists(%p)=%d, other.isExists(%p)=%d\n",
	// 				// itOther,isExists(itOther),itCur,other.isExists(itCur));
	// 			return false;
	// 		}
	// 		// if (isOrderedBefore(itCur, itOther) || isOrderedBefore(itOther, itCur) ||
	// 		// 	other.isOrderedBefore(itCur, itOther) ||
	// 		// 	other.isOrderedBefore(itOther, itCur)) 
	// 		// 	return false;
	// 	}
	// }
	// fprintf(stderr, "consistent\n");
	return true;
}


// domain-level feasibility checking
bool PartialOrder::isFeasible(Z3Minimal &zHelper, PartialOrder &other, llvm::Instruction *interfInst, llvm::Instruction *curInst) {
	// curInst should not be ordered before any inst in interferring domain
	for (auto it:other) {
		if (isSeqBefore(curInst, it.first)) return false;
	}
	// interfInst should not be ordered before any inst in current domain
	for (auto it:order) {
		if (isSeqBefore(interfInst, it.first)) return false;
	}
	return true;
}


// Removes inst from the element of the set. It should also remove
// all (x, inst) and (inst, x) pair from order for all possible 
// values of x
bool PartialOrder::remove(llvm::Instruction* inst) {
	// if (llvm::AtomicRMWInst *rmwInst = llvm::dyn_cast<llvm::AtomicRMWInst>(inst))
	// 	return true;
	for (auto it=order.begin(); it!=order.end(); ++it) {
		it->second.erase(inst);
	}
	order[inst].clear();
	order.erase(inst);
	// if (llvm::AtomicRMWInst *rmwInst = llvm::dyn_cast<llvm::AtomicRMWInst>(inst))
	// 	rmws.erase(inst);
	
	return true;
}

// get the last instructions in Partial Order
// inst \in lasts iff nEa (inst, a) \in order
unordered_set<llvm::Instruction*> PartialOrder::getLasts() {
	unordered_set<llvm::Instruction*> lasts;
	// iterate over all instructions in order
	for (auto it:order) {
		// if nothing is ordered after this, insert it into lasts
		if (it.second.empty())
			lasts.insert(it.first);
	}
	return lasts;
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


bool PartialOrder::addInst(Z3Minimal &zHelper, llvm::Instruction *inst) {
	for (auto it=order.begin(); it!=order.end(); ) {
		llvm::Instruction* instItr = it->first; ++it;
		if (instItr != inst && isSeqBefore(inst, instItr)) {
			// Nothing to add, a newer instruction is already there
			return true;
		}
		else if (instItr != inst && isSeqBefore(instItr, inst)) {
			// all the instructions ordered before instItr, should also
			// be ordered before instItr
			makeTransitiveOrdering(instItr, inst, order.end());
			
			// remove older instrutcion
			remove(instItr);
		}
	}
	auto findInst = order.find(inst);
	// add (inst,<empty>) in order 
	if (findInst == order.end()) {
		set<llvm::Instruction*> emptyset {};
		order[inst] = emptyset;
	}
	return true;
}

void PartialOrder::copy (const PartialOrder &copyFrom) {
	order = copyFrom.order;
	rmws = copyFrom.rmws;
}

string PartialOrder::toString() {
	std::stringstream ss;
	for (auto itFrom: order){
		ss << itFrom.first << " ---> " ;
		for (auto itTo: itFrom.second) {
			ss << itTo << ", ";
		}
		ss << ";\t";
	}
	// cout << ss.str() << "\n";
	return ss.str();
}

bool PartialOrder::operator==(const PartialOrder &other) const {
	return order == other.order;
}

bool PartialOrder::operator<(const PartialOrder &other) const {
	// TODO: Might need to change this. If instructions are disjoint,
	// this will not determine any order between the two POs. 
	// As per our definition, sb should play a role here.
	return order < other.order;
}

map<llvm::Instruction*, set<llvm::Instruction*>>::iterator PartialOrder::begin() {
	return order.begin();
}

map<llvm::Instruction*, set<llvm::Instruction*>>::iterator PartialOrder::end() {
	return order.end();
}
