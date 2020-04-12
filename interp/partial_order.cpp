#include "partial_order.h"

// Adds (from, to) to order if not already.  from-->to
// Since partial order can't be cyclic, if (to, from) are already
// in order, returns false. The behavior might be undefined 
// in this case. Else add (from, to) and returns true
void PartialOrder::addOrder(const InstNum &from, const InstNum &to) {
	// fprintf(stderr, "addOrder %p --> %p\n", from, to);
	
	if (isOrderedBefore(from, to)) return;
	// if (isOrderedBefore(to, from)) return false;
	assert(!isOrderedBefore(to, from) && "adding the order will break anti-symmetry");

	// find instNum of from and to
	
	// find 'to' in ordering
	auto toItr = order.find(to);
	// if 'to' does not exist, add an (to,<empty>) in order 
	// if (toItr == order.end()) {
	// 	set<InstNum> emptyset {};
	// 	order[to] = emptyset;
	// }

	// Check if some inst sequenced before 'from' or 'to' is in
	// the order. If yes, remove the older one.
	// OPT: The check is required only if 'from' or 'to' are not 
	// in the order already.
	for (auto it=order.begin(); it!=order.end(); ) {
		InstNum inst = it->first; ++it;
		// fprintf(stderr, "Checking SB %p-->%p, %p-->%p\n", inst,to,inst,from);
		if (inst != to && inst != from) {
			if (inst.isSeqBefore(to)) {
				// fprintf(stderr, "isseqbefore inst %p to %p\n", inst, to);
				addOrder(inst, to); 
				remove(inst);
			}
			else if (to.isSeqBefore(inst)) {
				// fprintf(stderr, "isseqbefore to %p inst %p\n",to, inst);
				addOrder(from, inst);
				return;
			}
			if (inst.isSeqBefore(from)) {
				// fprintf(stderr, "isseqbefore inst %p from %p\n", inst, from);
				addOrder(inst, from); 
				remove(inst);
			}
			else if (from.isSeqBefore(inst)) {
				makeTransitiveOrdering(from, to, toItr);
				remove(from);
				// fprintf(stderr, "isseqbefore from %p inst %p\n ", from, inst);
				addInst(to);
				return;
			}
		}
	}

	// set transitive ordering from-->to
	makeTransitiveOrdering(from, to, toItr);
}

// Adds inst such that Va \in order, (a, inst) \in order.
// Returns false if inst already exists in order
void PartialOrder::append(const InstNum &newinst) {
	// cout << "appending Partial order " << newinst << "\n";
	// If the 'newinst' is already in the order, there should 
	// not be any instruction ordered after it. Otherwise, 
	// it cannot be appended.
	auto searchInst = order.find(newinst);
	assert((searchInst == order.end() || searchInst->second.empty()) && "some instruction is ordered after the inst to be appended");

	// Check if some inst sequenced before 'inst' in order. 
	// If yes, remove the older one.
	for (auto it=order.begin(); it!=order.end(); ) {
		if (it->first.isSeqBefore(newinst)) {auto ittmp = it++; remove((ittmp->first));}
		else {
			// add newinst at in 'to' of it
			it->second.insert(newinst);
			it++;
		}
	}
	order.emplace(newinst, unordered_set<InstNum>());
	if(isRMWInst(newinst)) {
		rmws.insert(newinst);
	}
}

// Joins two partial orders maintaing the ordering relation in both
// If this is not possible (i.e. joining will result in cycle), 
// returns false
void PartialOrder::join(const PartialOrder &other) {
	// fprintf(stderr, "joining\n");
	// fprintf(stderr, "%s\t and \t%s\n", toString().c_str(), other.toString().c_str());
	for (auto fromItr=other.begin(); fromItr!=other.end(); ++fromItr) {
		// fprintf(stderr, "%p ",fromItr->first);
		if (fromItr->second.empty() && order.find(fromItr->first) == order.end()){
			addInst(fromItr->first);
		}
		for (auto toItr=fromItr->second.begin(); toItr!=fromItr->second.end(); ++toItr) {
			addOrder(fromItr->first, *toItr);
		}
		// if (isRMWInst(fromItr->first))
		// 	rmws.insert(fromItr->first);
	}
	// fprintf(stderr, "after join %s\n", toString().c_str());
}

// checks if (inst1, inst2) \in order
bool PartialOrder::isOrderedBefore(const InstNum &inst1, const InstNum &inst2) const {
	// if (!isExists(inst1)) return false;
	auto searchInst1 = order.find(inst1);
	if (searchInst1 == order.end()) return false;
	auto searchInst2 = searchInst1->second.find(inst2);
	if (searchInst2 == searchInst1->second.end()) return false;
	else return true;
}

// checks if inst is a part of this partial order
bool PartialOrder::isExists(const InstNum &inst) const {
	auto search = order.find(inst);
	if (search == order.end()) return false;
	else return true;
}

// checks if the partial order other is consistent with this partial order
// Two parial orders are consistent only if  Va.Vb (a,b) \in order 
// (b,a) \notin other.order, or viceversa
bool PartialOrder::isConsistent(const PartialOrder &other) {
	for (auto itFrom:other) {
		for (auto itTo:itFrom.second) {
			// if ordering is conistent 
			if (isOrderedBefore(itTo, itFrom.first)) return false;
		}
	}
	return true;
}

bool PartialOrder::isConsistentRMW(const PartialOrder &other) {
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
	for (auto itCur: rmws) {
		for (auto itOther: other.rmws) {
			if (!(isExists(itOther) || other.isExists(itCur))) {
				// fprintf(stderr, "not consistent, isExists(%p)=%d, other.isExists(%p)=%d\n",
					// itOther,isExists(itOther),itCur,other.isExists(itCur));
				return false;
			}
			else if (!(isOrderedBefore(itCur, itOther) || isOrderedBefore(itOther, itCur) ||
				other.isOrderedBefore(itCur, itOther) ||
				other.isOrderedBefore(itOther, itCur)))
				return false;
		}
	}
	// fprintf(stderr, "consistent\n");
	return true;
}


// domain-level feasibility checking
bool PartialOrder::isFeasible(const PartialOrder &other, InstNum &interfInst, InstNum &curInst) {
	// curInst should not be ordered before any inst in interferring domain
	for (auto it:other) {
		if (curInst.isSeqBefore(it.first)) return false;
	}
	// interfInst should not be ordered before any inst in current domain
	for (auto it:order) {
		if (interfInst.isSeqBefore(it.first)) return false;
	}
	return true;
}

// Removes inst from the element of the set. It should also remove
// all (x, inst) and (inst, x) pair from order for all possible 
// values of x
void PartialOrder::remove(const InstNum &inst) {
	// if (isRMWInst(inst)) return true;
	for (auto it=order.begin(); it!=order.end(); ++it) {
		it->second.erase(inst);
	}
	order[inst].clear();
	order.erase(inst);
	// if (isRMWInst(inst))
	// 	rmws.erase(inst);
}

// get the last instructions in Partial Order
// inst \in lasts iff nEa (inst, a) \in order
void PartialOrder::getLasts(unordered_set<InstNum> &lasts) const {
	// iterate over all instructions in order
	for (auto it:order) {
		// if nothing is ordered after this, insert it into lasts
		if (it.second.empty())
			lasts.insert(it.first);
	}
}

// update ordering relation to get transitivity
void PartialOrder::makeTransitiveOrdering (const InstNum &from, const InstNum &to, 
	std::unordered_map<InstNum, std::unordered_set<InstNum>>::iterator toItr
){
	// check for anti-symmetry
	assert(!isOrderedBefore(to, from) && "Anti symmetry chcek in PO failed");
	// if (isOrderedBefore(to, from)) return false;

	// insert 'to' in set of instrcutions partially ordered with 'from'
	if (order[from].insert(to).second == false) {
		// 'to' was already added in the ordering. No need to update ordering furthur
		return;
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
			makeTransitiveOrdering(it->first, to, toItr);
		}
	}
}

void PartialOrder::addInst(const InstNum &inst) {
	for (auto it=order.begin(); it!=order.end(); ) {
		InstNum instItr = it->first; ++it;
		if (instItr != inst && inst.isSeqBefore(instItr)) {
			// Nothing to add, a newer instruction is already there
			return ;
		}
		else if (instItr != inst && instItr.isSeqBefore(inst)) {
			// all the instructions ordered before instItr, should also
			// be ordered before instItr
			makeTransitiveOrdering(instItr, inst, order.end());
			// remove older instruction
			remove(instItr);
		}
	}
	auto findInst = order.find(inst);
	// add (inst,<empty>) in order 
	if (findInst == order.end()) {
		unordered_set<InstNum> emptyset {};
		order[inst] = emptyset;
	}
	if (isRMWInst(inst))
		rmws.insert(inst);
	return;
}

bool PartialOrder::isRMWInst(const InstNum &inst) {
	if (llvm::dyn_cast<llvm::AtomicRMWInst>(getInstByInstNum(inst))) {
		// fprintf(stderr, "rmw inst\n");
		return true;
	}
	else return false;
}

string PartialOrder::toString() const {
	std::stringstream ss;
	for (auto itFrom=order.begin(); itFrom!=order.end(); ++itFrom) {
		// fprintf(stderr, "in outer for\n");
		// fprintf(stderr, "size of second %lu\n", itFrom->second.size());
		ss << itFrom->first.toString() << " ---> " ;
		for (auto itTo: itFrom->second) {
			// fprintf(stderr, "in inner for\n");
			ss << itTo.toString() << ", ";
		}
		ss << ";\t";
	}
	ss << "\tRMWs: ";
	for (auto it=rmws.begin(); it!=rmws.end(); it++) {
		// fprintf(stderr, "in rmws loop\n");
		ss << it->toString() << ",";
	}
	return ss.str();
}

bool PartialOrder::operator==(const PartialOrder &other) const {
	return order == other.order;
}

// void PartialOrder::operator= (const PartialOrder &other) {
// 	fprintf(stderr, "assigning from PO class\n");
// 	order = other.order;
// 	rmws  = other.rmws;
// 	fprintf(stderr, "assigned from PO class\n");
// }

// bool PartialOrder::operator<(const PartialOrder &other) const {
// 	// TODO: Might need to change this. If instructions are disjoint,
// 	// this will not determine any order between the two POs. 
// 	// As per our definition, sb should play a role here.
// 	return order < other.order;
// }

void PartialOrder::copy (const PartialOrder &copyFrom) {
	order = copyFrom.order;
	rmws = copyFrom.rmws;
}

void PartialOrder::clear() {
	order.clear();
	rmws.clear();
}

unordered_map<InstNum, unordered_set<InstNum>>::const_iterator PartialOrder::begin() const {
	return order.begin();
}

unordered_map<InstNum, unordered_set<InstNum>>::const_iterator PartialOrder::end() const {
	return order.end();
}



//////////////////////////////////////////
//      class PartialOrderWrapper       //
//////////////////////////////////////////

unordered_set<PartialOrder*> PartialOrderWrapper::allPO;

const PartialOrder& PartialOrderWrapper::addToSet(PartialOrder *po, bool &isAlreadyExist) {
	// auto searchPO = allPO.find(*po);
	// auto searchPO = allPO.find(po);
	// if (searchPO != allPO.end()) {
	// 	return **searchPO;
	// }
	// else {
	auto inserted = allPO.insert(po);
	isAlreadyExist = inserted.second;
	return **(inserted.first);
	// }
	// fprintf(stderr, "done. returning\n");
}

PartialOrder PartialOrderWrapper::append(const PartialOrder &curPO, InstNum &inst) {
	PartialOrder *tmpPO = new PartialOrder();
    tmpPO->copy(curPO);
    tmpPO->append(inst);
    // fprintf(stderr, "after append: %s\n", tmpPO->toString().c_str());
	bool isAlreadyExist;
	auto po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	else return po;
}

PartialOrder PartialOrderWrapper::addOrder(PartialOrder &curPO, InstNum &from, InstNum &to) {
	PartialOrder *tmpPO = new PartialOrder();
    tmpPO->copy(curPO);
    tmpPO->addOrder(from, to);
    // fprintf(stderr, "after addOrder: %s\n", tmpPO->toString().c_str());
    bool isAlreadyExist;
	auto po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	else return po;
}

PartialOrder PartialOrderWrapper::join(PartialOrder &curPO, const PartialOrder &other) {
	PartialOrder *tmpPO = new PartialOrder();
    tmpPO->copy(curPO);
    tmpPO->join(other);
    // fprintf(stderr, "after join: %s\n", tmpPO->toString().c_str());
    bool isAlreadyExist;
	auto po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	else return po;
}

PartialOrder PartialOrderWrapper::remove(PartialOrder &curPO, InstNum &inst) {
	PartialOrder *tmpPO = new PartialOrder();
    tmpPO->copy(curPO);
    tmpPO->remove(inst);
    // fprintf(stderr, "after remove: %s\n", tmpPO->toString().c_str());
    bool isAlreadyExist;
	auto po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	else return po;
}

bool PartialOrderWrapper::hasInstance (PartialOrder &po) {
	auto searchPO = allPO.find(&po);
	if (searchPO != allPO.end()) {
		return true;
	}
	else {
		return false;
	}
}

void PartialOrderWrapper::printAllPO() {
	fprintf(stderr, "AllPO:\n");
	int i=0;
	for (auto it: allPO) {
		fprintf(stderr, "%d: %s\n",i , it->toString().c_str());
		i++;
	}
}

void PartialOrderWrapper::clearAllPO() {
	for (auto it: allPO) {
		it->clear();
	}
	allPO.clear();
	
}
