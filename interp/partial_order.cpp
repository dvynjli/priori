#include "partial_order.h"

// Adds (from, to) to order if not already.  from-->to
// Since partial order can't be cyclic, if (to, from) are already
// in order, returns false. The behavior might be undefined 
// in this case. Else add (from, to) and returns true
void PartialOrder::addOrder(const InstNum &from, const InstNum &to) {
	//fprintf(stderr, "addOrder %p --> %p\n", from, to);
	
	if (isDeletableInst(from) && isOrderedBefore(from, to)) return;
	// if (isOrderedBefore(to, from)) return false;
	// if (isOrderedBefore(to, from)) {
	// 	fprintf(stderr, "CurPO: %s\n", toString().c_str());
	// 	fprintf(stderr, "adding: %s-->%s\n", from.toString().c_str(), to.toString().c_str());
	// }
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
	// if (deleteOlder && (Precision == P0 || (!isRMWInst(from) && !isRMWInst(to)))) {
	// if (deleteOlder) {
		for (auto it=order.begin(); it!=order.end(); ) {
			InstNum inst = it->first; ++it;
			// fprintf(stderr, "Checking SB %p-->%p, %p-->%p\n", inst,to,inst,from);
			if (inst != to && inst != from) {
				if (inst.isSeqBefore(to)) {
					// fprintf(stderr, "isseqbefore inst %s to %s\n", inst.toString().c_str(), to.toString().c_str());
					if (isDeletableInst(inst) || Precision==P0) { 
						addOrder(inst, to); 
						remove(inst);
					}
				}
				else if ((isDeletableInst(to) || Precision==P0) && to.isSeqBefore(inst)) {
					// fprintf(stderr, "isseqbefore to %p inst %p\n",to, inst);
					// fprintf(stderr, "isseqbefore to %s inst %s\n", to.toString().c_str(), inst.toString().c_str());
					addOrder(from, inst);
					return;
				}
				if (inst.isSeqBefore(from)) {
					// fprintf(stderr, "isseqbefore inst %p from %p\n", inst, from);
					// fprintf(stderr, "isseqbefore inst %s from %s\n", inst.toString().c_str(), from.toString().c_str());
					if (isDeletableInst(inst) || Precision==P0) {
						addOrder(inst, from); 
						remove(inst);
					}
				}
				else if ((isDeletableInst(from) || Precision==P0) && from.isSeqBefore(inst)) {
					makeTransitiveOrdering(from, to, toItr);
					/* if (!isRMWInst(from))*/ remove(from);
					// fprintf(stderr, "isseqbefore from %p inst %p\n ", from, inst);
					// fprintf(stderr, "isseqbefore from %s inst %s\n", from.toString().c_str(), inst.toString().c_str());
					addInst(to);
					return;
				}
			}
		}
	//}

	// set transitive ordering from-->to
	makeTransitiveOrdering(from, to, toItr);
}

// Adds inst such that Va \in order, (a, inst) \in order.
// Returns false if inst already exists in order
void PartialOrder::append(const InstNum &newinst) {
	// fprintf(stderr, "appedning %s to PO: %s\n",newinst.toString().c_str(), toString().c_str());
	// If the 'newinst' is already in the order, there should 
	// not be any instruction ordered after it. Otherwise, 
	// it cannot be appended.
	// fprintf(stderr, "deleteOlder: %d\n",deleteOlder);
	auto searchInst = order.find(newinst);
	assert((searchInst == order.end() || searchInst->second.empty()) && "some instruction is ordered after the inst to be appended");

	// Check if some inst sequenced before 'inst' in order. 
	// If yes, remove the older one.
	for (auto it=order.begin(); it!=order.end(); ) {
		// fprintf(stderr,"checking for %s\n",it->first.toString().c_str());
		if (it->first == newinst) {
			it++; continue;
		}
		if (it->first.isSeqBefore(newinst) && (Precision==P0 || isDeletableInst(it->first))) {
			// fprintf(stderr, "seqbefore new inst. deleting\n");
			auto ittmp = it++; 
			remove((ittmp->first));
		}
		else {
			// add newinst at in 'to' of it
			// fprintf(stderr,"not seq before new inst. appending\n");
			it->second.insert(newinst);
			it++;
		}
	}
	order.emplace(newinst, unordered_set<InstNum>());
	if(Precision>P0 && (isRMWInst(newinst) || isLockInst(newinst) || isUnlockInst(newinst))) {
		rmws.insert(newinst);
	}
	// fprintf(stderr, "after append: %s\n", toString().c_str());
}

// Joins two partial orders maintaing the ordering relation in both
// If this is not possible (i.e. joining will result in cycle), 
// returns false
void PartialOrder::join(const PartialOrder &other) {
	// fprintf(stderr, "PO joining\n");
	// fprintf(stderr, "%s\t and \t%s\n", toString().c_str(), other.toString().c_str());
	for (auto fromItr=other.begin(); fromItr!=other.end(); ++fromItr) {
		// fprintf(stderr, "%s ",fromItr->first.toString().c_str());
		if (fromItr->second.empty() && order.find(fromItr->first) == order.end()){
			addInst(fromItr->first);
		}
		for (auto toItr=fromItr->second.begin(); toItr!=fromItr->second.end(); ++toItr) {
			addOrder(fromItr->first, *toItr);
		}
		// if (isRMWInst(fromItr->first))
		// 	rmws.insert(fromItr->first);
	}
	// fprintf(stderr, "PO after join %s\n", toString().c_str());
	if (Precision>P0) rmws.insert(other.rmws.begin(), other.rmws.end());
}

// checks if (inst1, inst2) \in order
bool PartialOrder::isOrderedBefore(const InstNum &inst1, const InstNum &inst2) const {
	// fprintf(stderr, "checking isOrdBefore %s, %s: ", inst1.toString().c_str(), inst2.toString().c_str());
	// if (!isExists(inst1)) return false;
	if (inst1==inst2) {
		// fprintf(stderr, "1\n"); 
		return true;
	}
	if (inst1.isSeqBefore(inst2)) return true;
	auto searchInst1 = order.find(inst1);
	if (searchInst1 == order.end()) {
		// search if some inst seqeunces after inst1 is in order
		for (auto it=order.begin(); it!=order.end(); it++) {
			if (inst1.isSeqBefore(it->first)) {
				searchInst1 = it;
				break;
			}
		}
		// no such inst found, hence inst1 or any inst sequenced after inst1 is not in order
		if (searchInst1 == order.end()) {
			// fprintf(stderr, "0"); 
			return false;
		}
	}
	auto searchInst2 = searchInst1->second.find(inst2);
	if (searchInst2 == searchInst1->second.end()) {
		// fprintf(stderr, "0"); 
		return false;
	}
	else {
		// fprintf(stderr, "1"); 
		return true;
	}
}

// checks if inst is a part of this partial order
bool PartialOrder::isExists(const InstNum &inst) const {
	auto search = order.find(inst);
	if (search == order.end()) return false;
	else return true;
}

// checks if the partial order other is consistent with this partial order
// Two parial orders are consistent only if  Va.Vb (a,b) \in order 
// (c,d) \notin other.order such that b--sb-->c and d--sb-->a
bool PartialOrder::isConsistent(const PartialOrder &other) const {
	for (auto itFrom=order.begin(); itFrom!=order.end(); itFrom++) {	// a
		for (auto itTo=itFrom->second.begin(); itTo!=itFrom->second.end(); itTo++) { // b
			for (auto itOtherFrom=other.begin(); itOtherFrom!=other.end(); itOtherFrom++) { // c
				if (itTo->isSeqBefore(itOtherFrom->first)) {
					// we found (a,b) \in p1, (c,-) \in p2 such that b--sb-->c
					for (auto itOtherTo=itOtherFrom->second.begin(); itOtherTo!=itOtherFrom->second.end(); itOtherTo++) 	// d
						if (itOtherTo->isSeqBefore(itFrom->first)) return false;
				}
			} 
		}
	}
	return true;
}

bool PartialOrder::isConsRMWP2(const PartialOrder &other) const {
	for (auto itCur=rmws.begin(); itCur!=rmws.end(); itCur++) {
		for (auto itOther=other.rmws.begin(); itOther!=other.rmws.end(); itOther++) {
			if (itCur == itOther) continue;
			if (!(isExists(*itOther) || other.isExists(*itCur))) {
				// fprintf(stderr, "not consistent, isExists(%p)=%d, other.isExists(%p)=%d\n",
					// itOther,isExists(itOther),itCur,other.isExists(itCur));
				return false;
			}
			else if (!(isOrderedBefore(*itCur, *itOther) || 
				isOrderedBefore(*itOther, *itCur) ||
				other.isOrderedBefore(*itCur, *itOther) ||
				other.isOrderedBefore(*itOther, *itCur)))
				return false;
		}
	}
	// fprintf(stderr, "consistent\n");
	return true;
}

bool PartialOrder::isConsRMWP3(const PartialOrder &other) const {
	// unordered_set<InstNum> otherLasts;
	// other.getLasts(otherLasts);
	// fprintf(stderr, "checking cons P3\n");
	for (auto otherFromIt=other.order.begin(); otherFromIt!=other.order.end(); otherFromIt++) {
		if (!isDeletableInst(otherFromIt->first)) {
			// fprintf(stderr, "from: %s -->", otherFromIt->first.toString().c_str());
			auto searchFromCur = order.find(otherFromIt->first);
			// if (searchFromCur == order.end() && otherLasts.find(otherFromIt->first)==otherLasts.end() ) {
			// 	// fprintf(stderr, "from not found in cur and not last\n");
			// 	// inst from other not found in cur and not last in other
			// 	return false;
			// }
			// for (auto otherToIt=otherFromIt->second.begin(); otherToIt!=otherFromIt->second.end(); otherToIt++) {
			// 	// fprintf(stderr, "--> %s\n", otherToIt->toString().c_str());
			// 	auto searchToCur = searchFromCur->second.find(*otherToIt);
			// 	if (searchToCur==searchFromCur->second.end() && otherLasts.find(*otherToIt)==otherLasts.end()) {
			// 		// fprintf(stderr, "To not found in cur\n");
			// 		// ordering from other not found in cur and not last in other
			// 		return false;
			// 	}
			// }
			if (searchFromCur == order.end()) {
				// if from inst of interf not found in cur
				// no inst ordered after it in interf should be in cur
				for (auto otherToIt=otherFromIt->second.begin(); otherToIt!=otherFromIt->second.end(); otherToIt++) {
					auto searchToCur = order.find(*otherToIt);
					if (searchToCur != order.end()) return false;
				}
			}
		}
	}
	for (auto curFromIt=order.begin(); curFromIt!=order.end(); curFromIt++) {
	 	if (!isDeletableInst(curFromIt->first)) {	// need to check only for RMWs
			auto searchFromOther = other.order.find(curFromIt->first);
			if (searchFromOther == order.end()) {
				return false;
			}
			for (auto curToIt=curFromIt->second.begin(); curToIt!=curFromIt->second.end(); curToIt++) {
				auto searchToOther = searchFromOther->second.find(*curToIt);
				if (searchToOther==searchFromOther->second.end())
					return false;
			}
		}			
	}	
	return true;
}

bool PartialOrder::isConsistentRMW(const PartialOrder &other) const {
	if (Precision == P0) {return true;}
	else if (Precision == P1) {return true;}
	else if (Precision == P2) {
		return isConsRMWP2(other);
	}
	else {
		return isConsRMWP2(other) && isConsRMWP3(other);
	}
}


// domain-level feasibility checking
bool PartialOrder::isFeasible(const PartialOrder &other, InstNum &interfInst, InstNum &curInst) const {
	// curInst should not be ordered before any inst in interferring domain
	for (auto it=other.begin(); it!=other.end(); it++) {
		if (curInst.isSeqBefore(it->first)) return false;
	}
	// interfInst should not be ordered before any inst in current domain
	for (auto it=order.begin(); it!=order.end(); it++) {
		if (interfInst.isSeqBefore(it->first)) return false;
	}
	return true;
}

// Removes inst from the element of the set. It should also remove
// all (x, inst) and (inst, x) pair from order for all possible 
// values of x
void PartialOrder::remove(const InstNum &inst) {
	assert((isDeletableInst(inst) || Precision==P0) && "Removing inst rmw/lock/unlock from PO");
	if (Precision >= P3 && !isDeletableInst(inst)) return ;
	for (auto it=order.begin(); it!=order.end(); ++it) {
		it->second.erase(inst);
	}
	order[inst].clear();
	order.erase(inst);
	if (Precision == P0 && isDeletableInst(inst))
		rmws.erase(inst);
	else if (Precision == P1) {
		//TODO
	}
}

bool PartialOrder::isDeletableInst(const InstNum &inst) const {
	if (isRMWInst(inst) || isLockInst(inst) || isUnlockInst(inst)) return false;
	else return true;
}

// get the last instructions in Partial Order
// inst \in lasts iff nEa (inst, a) \in order
void PartialOrder::getLasts(unordered_set<InstNum> &lasts) const {
	// iterate over all instructions in order
	for (auto it=order.begin(); it!=order.end(); it++) {
		// if nothing is ordered after this, insert it into lasts
		if (it->second.empty())
			lasts.insert(it->first);
	}
}

// update ordering relation to get transitivity
void PartialOrder::makeTransitiveOrdering (const InstNum &from, const InstNum &to, 
	std::unordered_map<InstNum, std::unordered_set<InstNum>>::iterator toItr
){
	// check for anti-symmetry
	if (to == from) return;
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
	// if (deleteOlder) {
		for (auto it=order.begin(); it!=order.end(); ) {
			InstNum instItr = it->first; ++it;
			if ((isDeletableInst(inst) || Precision==P0) && instItr != inst && inst.isSeqBefore(instItr)) {
				// Nothing to add, a newer instruction is already there
				return ;
			}
			else if (instItr != inst && instItr.isSeqBefore(inst)) {
				// all the instructions ordered before instItr, should also
				// be ordered before instItr
				makeTransitiveOrdering(instItr, inst, order.end());
				// remove older instruction
				if (isDeletableInst(instItr) || Precision==P0) remove(instItr);
			}
		}
	// }
	auto findInst = order.find(inst);
	// add (inst,<empty>) in order 
	if (findInst == order.end()) {
		unordered_set<InstNum> emptyset {};
		order[inst] = emptyset;
	}
	if (Precision>P0 && (isRMWInst(inst) || isLockInst(inst) || isUnlockInst(inst)))
		rmws.insert(inst);
}

bool PartialOrder::lessThan(const PartialOrder &other) const {
	if (Precision == P0) {}
	else if (Precision == P1) {}
	else /*if (Precision == P2)*/ {
		for (auto curIt=rmws.begin(); curIt!=rmws.end(); curIt++) {
			auto searchRmw = other.rmws.find(*curIt);
			if (searchRmw == other.rmws.end()) {
				return false;
			}
		}
	}
	// else if (Precision == P3) {}
	for (auto curIt=order.begin(); curIt!=order.end(); curIt++) {
		bool foundBiggerInst = false;
		for (auto otherIt=other.begin(); otherIt!=other.end(); otherIt++) {
			if (curIt->first.isSeqBefore(otherIt->first)) {
				foundBiggerInst = true;
				// find if all ordered after insts are also less than some inst in other
				for (auto curItTo=curIt->second.begin();curItTo!=curIt->second.end(); curItTo++) {
					foundBiggerInst = false;
					for (auto otherItTo=otherIt->second.begin(); otherItTo!=otherIt->second.end(); otherItTo++) {
						if (curItTo->isSeqBefore(*otherItTo)) {
							foundBiggerInst = true;
							break;
						}
					}
					if (!foundBiggerInst) return false;
				}
				break;
			}
		}
		if (!foundBiggerInst) return false;
	}
	return true;
}

string PartialOrder::toString() const {
	std::stringstream ss;
	// fprintf(stderr, "deleteOlder: %d\n",deleteOlder);
	for (auto itFrom=order.begin(); itFrom!=order.end(); ++itFrom) {
	// 	fprintf(stderr, "in outer for\n");
	// 	fprintf(stderr, "size of second %lu\n", itFrom->second.size());
		ss << itFrom->first.toString() << " ---> " ;
	 	for (auto itTo: itFrom->second) {
	 		// fprintf(stderr, "in inner for\n");
	 		ss << itTo.toString() << ", ";
	 	}
	 	ss << ";\t";
	 }
	ss << "del: " << deleteOlder;
	// ss << "\tRMWs: ";
	// for (auto it=rmws.begin(); it!=rmws.end(); it++) {
	// 	// fprintf(stderr, "in rmws loop\n");
	// 	ss << it->toString() << ",";
	// }
	//fprintf(stderr, "returning from toString\n");
	return ss.str();
}

bool PartialOrder::operator==(const PartialOrder &other) const {
	return deleteOlder == other.deleteOlder && order == other.order;
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
	deleteOlder = copyFrom.deleteOlder;
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

unordered_set<
	PartialOrder*, 
	hashPOPointer, 
	comparePOPointer> PartialOrderWrapper::allPO;

unordered_map<
	pair<const PartialOrder*, const PartialOrder*>, 
	const PartialOrder*,
	hashPOPair,
	comparePOPair> PartialOrderWrapper::cachedJoin;

unordered_map<
	pair<const PartialOrder*, const PartialOrder*>, 
	const PartialOrder*,
	hashPOPair,
	comparePOPair> PartialOrderWrapper::cachedMeet;
	
unordered_map<
	pair<const PartialOrder*, const InstNum*>, 
	const PartialOrder*,
	hashPOInstNumPair,
	comparePOInstNumPair> PartialOrderWrapper::cachedAppend;

const PartialOrder& PartialOrderWrapper::addToSet(PartialOrder *po, bool &isAlreadyExist) {
	// auto searchPO = allPO.find(*po);
	// auto searchPO = allPO.find(po);
	// if (searchPO != allPO.end()) {
	// 	return **searchPO;
	// }
	// else {
	// fprintf(stderr, "in addToSet:%p:::%s\n", po, po->toString().c_str());
	// printAllPO();
	auto inserted = allPO.insert(po);
	isAlreadyExist = !inserted.second;
	// fprintf(stderr, "found in allPO:%d\n",isAlreadyExist);
	/// printAllPO();
	return **(inserted.first);
	// }
	// fprintf(stderr, "done. returning\n");
}

PartialOrder& PartialOrderWrapper::getEmptyPartialOrder(bool delOlder) {
	// fprintf(stderr, "called getEmptyPO with del=%d\n",delOlder);
	// printAllPO();
	PartialOrder *tmpPO = new PartialOrder(delOlder);
	bool isAlreadyExist;
	const PartialOrder& po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	// fprintf(stderr, "added in allPO:%p %s", &po, po.toString().c_str());
	// printAllPO();
	return (PartialOrder&) po;
}

PartialOrder& PartialOrderWrapper::append(const PartialOrder &curPO, InstNum &inst) {
	// fprintf(stderr, "appending: %s <| %p %s\n",curPO.toString().c_str(), inst, inst.toString().c_str());
	// printAllPO();
	// printAppendCache();
	const pair<const PartialOrder*, const InstNum*> poInstNumPair = make_pair(&curPO, &inst);
	auto searchInCached = cachedAppend.find(poInstNumPair);
	if (searchInCached != cachedAppend.end()) {
		// fprintf(stderr, "found in cache. Returning %p %s\n",searchInCached->second, searchInCached->second->toString().c_str());
		// fprintf(stderr, "value: %s\n", searchInCached->second->toString().c_str());
		return (PartialOrder&) *searchInCached->second;
	}
	PartialOrder *tmpPO = new PartialOrder(curPO.deleteOlder);
    tmpPO->copy(curPO);
    tmpPO->append(inst);
    // fprintf(stderr, "after append: %p %s\n", tmpPO, tmpPO->toString().c_str());
	bool isAlreadyExist;
	const PartialOrder& po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	cachedAppend[poInstNumPair] = &po;
	// fprintf(stderr, "after append %p:   %s\n",&po, po.toString().c_str());
	// printAllPO();
	// printAppendCache();
	return (PartialOrder&) po;
}

PartialOrder& PartialOrderWrapper::addOrder(const PartialOrder &curPO, InstNum &from, InstNum &to) {
	// fprintf(stderr, "addOrder to %s adding %s -> %s\n", 
	// 		curPO.toString().c_str(), from.toString().c_str(), 
	// 		to.toString().c_str());
	PartialOrder *tmpPO = new PartialOrder(curPO.deleteOlder);
    tmpPO->copy(curPO);
    tmpPO->addOrder(from, to);
    // fprintf(stderr, "after addOrder: %s\n", tmpPO->toString().c_str());
    bool isAlreadyExist;
	const PartialOrder& po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	// fprintf(stderr, "after addOrder %s\n", po.toString().c_str());
	return (PartialOrder&)po;
}

void PartialOrderWrapper::printJoinCache() {
	for (auto it=cachedJoin.begin(); it!=cachedJoin.end(); it++) {
		fprintf(stderr, "%p %s join %p %s ==== %p %s\n", 
				it->first.first,
				it->first.first->toString().c_str(), 
				it->first.second,
				it->first.second->toString().c_str(),
				it->second,
				it->second->toString().c_str());
	}
}

PartialOrder& PartialOrderWrapper::join(const PartialOrder &curPO, const PartialOrder &other) {
	const pair<const PartialOrder*, const PartialOrder*> poPair = make_pair(&curPO, &other);
	auto searchInCached = cachedJoin.find(poPair);
	// printJoinCache();
	if (searchInCached != cachedJoin.end()) {
		// fprintf(stderr, "returning from cache %p %s\n",searchInCached->second, searchInCached->second->toString().c_str());
		return (PartialOrder&) *searchInCached->second;
	}
	PartialOrder *tmpPO = new PartialOrder(curPO.deleteOlder);
    tmpPO->copy(curPO);
    tmpPO->join(other);
    // fprintf(stderr, "after join: %s\n", tmpPO->toString().c_str());
    bool isAlreadyExist;
	const PartialOrder& po = addToSet(tmpPO, isAlreadyExist);
    if (isAlreadyExist) {
		delete tmpPO;
	}
	cachedJoin[poPair] = &po;
	// fprintf(stderr, "added to cache %p\n",&po);
	return (PartialOrder&)po;
}

PartialOrder& PartialOrderWrapper::meet(const PartialOrder &curPO, const PartialOrder &other) {
	// fprintf(stderr, "taking meet of %s and %s\n",curPO.toString().c_str(), other.toString().c_str());
	const pair<const PartialOrder*, const PartialOrder*> poPair = make_pair(&curPO, &other);
	auto searchInCached = cachedMeet.find(poPair);
	if (searchInCached != cachedMeet.end()) {
		return (PartialOrder&) *searchInCached->second;
	}

	PartialOrder *tmpPO = new PartialOrder(curPO.deleteOlder);
	for (auto curIt=curPO.begin(); curIt!=curPO.end(); curIt++) {
		for (auto otherIt=other.begin(); otherIt!=other.end(); otherIt++) {
			if (curIt->first.isSeqBefore(otherIt->first)) {
				// curIt should be part of meet
				tmpPO->addInst(curIt->first);
				// add the insts ordered after curIt->first
				for (auto curItTo=curIt->second.begin(); curItTo!=curIt->second.end(); curItTo++) {
					for (auto otherItTo=otherIt->second.begin(); otherItTo!=otherIt->second.end(); otherItTo++) {
						if (curItTo->isSeqBefore(*otherItTo)) {
							// fprintf(stderr, "calling addOrder %s to %s\n",curIt->first.toString().c_str(), curItTo->toString().c_str());
							tmpPO->addOrder(curIt->first, *curItTo);
							// fprintf(stderr, "added\n");
						}
						else if (otherItTo->isSeqBefore(*curItTo)) {
							// fprintf(stderr, "calling addOrder %s to %s\n",curIt->first.toString().c_str(), otherItTo->toString().c_str());
							tmpPO->addOrder(curIt->first, *otherItTo);
							// fprintf(stderr, "added\n");
						}
					}
				}
			}
			else if (otherIt->first.isSeqBefore(curIt->first)) {
				// otherIt should be part of meet
				tmpPO->addInst(otherIt->first);
				// add inst ordered after other->first
				for (auto curItTo=curIt->second.begin(); curItTo!=curIt->second.end(); curItTo++) {
					for (auto otherItTo=otherIt->second.begin(); otherItTo!=otherIt->second.end(); otherItTo++) {
						if (curItTo->isSeqBefore(*otherItTo)) {
							// fprintf(stderr, "calling addOrder %s to %s\n",otherIt->first.toString().c_str(), curItTo->toString().c_str());
							// fprintf(stderr, "PO so far: %s\n", tmpPO->toString().c_str());
							tmpPO->addOrder(otherIt->first, *curItTo);
							// fprintf(stderr, "added\n");
						}
						else if (otherItTo->isSeqBefore(*curItTo)) {
							// fprintf(stderr, "calling addOrder %s to %s\n",otherIt->first.toString().c_str(), otherItTo->toString().c_str());
							// fprintf(stderr, "PO so far: %s\n", tmpPO->toString().c_str());
							tmpPO->addOrder(otherIt->first, *otherItTo);
							// fprintf(stderr, "added\n");
						}
					}
				}
			}
			// else none of them should be in meet, skip
		}
	}

	// remove the rmws that are not in other
	for (auto curIt=curPO.rmws.begin(); curIt!=curPO.rmws.end(); curIt++) {
		auto searchRmw = other.rmws.find(*curIt);
		if (searchRmw != other.rmws.end()) {
			// add it to tmpPO
			tmpPO->rmws.insert(*curIt);
		}
	}

	// add to the set of all POs
	// fprintf(stderr, "after join: %s\n", tmpPO->toString().c_str());
	bool isAlreadyExist;
	const PartialOrder& po = addToSet(tmpPO, isAlreadyExist);
	if (isAlreadyExist) {
		delete tmpPO;
	}
	// fprintf(stderr, "meet is: %s\n",po.toString().c_str());
	cachedMeet[poPair] = &po;
	return (PartialOrder&)po;
}

// PartialOrder& PartialOrderWrapper::remove(const PartialOrder &curPO, InstNum &inst) {
// 	PartialOrder *tmpPO = new PartialOrder(curPO.deleteOlder);
//     tmpPO->copy(curPO);
//     if (tmpPO->isDeletableInst(inst) || Precision==P0) 
// 		tmpPO->remove(inst);
//     // fprintf(stderr, "after remove: %s\n", tmpPO->toString().c_str());
//     bool isAlreadyExist;
// 	const PartialOrder& po = addToSet(tmpPO, isAlreadyExist);
//     if (isAlreadyExist) {
// 		delete tmpPO;
// 	}
// 	return (PartialOrder&) po;
// }

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
	fprintf(stderr, "AllPO:%u\n",allPO.size());
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
