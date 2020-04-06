#ifndef _PO
#define _PO

#include "common.h"
#include "z3_handler.h"

class PartialOrder {
	// all the instructions ordered after a in partial order are given 
	// by order[a]. In other words, Va.Vb (a,b) \in po => (b in order[a]).
	// If an instruction inst is not ordered with respect to any 
	// instruction in partial order so far, order[inst] will be empty 
	// set and ~E x. inst \in order[x]
	unordered_map<InstNum, unordered_set<InstNum>> order;
	unordered_set<InstNum> rmws;

	// update ordering relation to get transitivity
	bool makeTransitiveOrdering(InstNum from, 
		InstNum to, 
		std::unordered_map<InstNum, std::unordered_set<InstNum>>::iterator toItr);

	// adds an instruction with nothing ordered after it ib the order,
	// while deleting the older instructions from the same thread
	bool addInst(Z3Minimal &zHelper, InstNum inst);
	bool isRMWInst(InstNum inst);

public:
	PartialOrder() :
		order(unordered_map<InstNum, unordered_set<InstNum>>()), 
		rmws(unordered_set<InstNum>()) {
		// order.clear(); 
		// fprintf(stderr, "PO cons called\n");
		}
	
	PartialOrder(const PartialOrder &po) : order(po.order), rmws(po.rmws) {};
	// Adds (from, to) to order if not already. 
	// Since partial order can't be cyclic, if (to, from) are already
	// in order, returns false. Else add (from, to) and returns true
	bool addOrder(Z3Minimal &zHelper, InstNum from, InstNum to);

	// Adds inst such that Va \in order, (a, inst) \in order.
	// Returns false if inst \in order and order[inst] != {}
	bool append(Z3Minimal &zHelper, InstNum inst);

	// Joins two partial orders maintaing the ordering relation in both
	// If this is not possible (i.e. joining will result in cycle), 
	// returns false
	bool join(Z3Minimal &zHelper, const PartialOrder &other);

	// checks if (inst1, inst2) \in order
	bool isOrderedBefore(InstNum inst1, InstNum inst2);

	// checks if inst is a part of this partial order
	bool isExists(InstNum inst);

	// checks if the partial order other is consistent with this partial order
	// Two parial orders are consistent only if Va.Vb (a,b) \in order
	// (b,a) \notin other.order, or wiseversa
	bool isConsistent(PartialOrder &other);

	bool isConsistentRMW(PartialOrder &other);

	// domain-level feasibility checking
	bool isFeasible(Z3Minimal &zHelper, PartialOrder &other, InstNum interfInst, InstNum curInst);

	// Removes inst from the element of the set. It should also remove
	// all (x, inst) and (inst, x) pair from order for all possible 
	// values of x
	bool remove(InstNum inst);

	// get the last instructions in Partial Order
	// inst \in lasts iff nEa (inst, a) \in order
	void getLasts(unordered_set<InstNum> &lasts);
	
	virtual bool operator== (const PartialOrder &other) const;
	// virtual bool operator<  (const PartialOrder &other) const;

	unordered_map<InstNum, unordered_set<InstNum>>::const_iterator begin() const;
	unordered_map<InstNum, unordered_set<InstNum>>::const_iterator end() const;

	void copy (const PartialOrder &copyFrom);

	string toString();
};

namespace std{
template<>
struct hash<PartialOrder> {
	size_t operator() (const PartialOrder &po) const {
		// return (hash<unsigned short>()(in.getTid()) ^ 
		auto it = po.begin();
		if (it == po.end())
			return hash<InstNum>()(InstNum(0,0));
		size_t curhash = hash<InstNum>()(it->first);
		it++;
		for (; it!=po.end(); it++) {
			curhash = curhash ^ hash<InstNum>()(it->first);
		}
		// fprintf(stderr, "returning hash\n");
		return curhash;
	}
};
}

class PartialOrderWrapper {
	PartialOrder &thisPO;

	const PartialOrder& addToSet(PartialOrder &po);

	
public:
	static unordered_set<PartialOrder> allPO;

	PartialOrderWrapper(PartialOrder *po) : thisPO(*po) {
		// PartialOrder *tmpPO = new PartialOrder(po);
		// fprintf(stderr, "cons called %p %p", &po, tmpPO);
		thisPO = addToSet(*po);
		// fprintf(stderr, "cons called thisPO %p\n", &thisPO);
		// fprintf(stderr, "done\n");
	}

	PartialOrderWrapper() = delete;

	PartialOrderWrapper(const PartialOrderWrapper &other) : thisPO(other.getPO()) {}
	
	void addOrder(Z3Minimal &zHelper, InstNum from, InstNum to);
	PartialOrderWrapper append(Z3Minimal &zHelper, InstNum inst);
	void join(Z3Minimal &zHelper, const PartialOrderWrapper &other);
	void remove(InstNum inst);
	
	bool isOrderedBefore(InstNum inst1, InstNum inst2);
	bool isExists(InstNum inst);
	bool isConsistent(PartialOrderWrapper &other);
	bool isConsistentRMW(PartialOrderWrapper &other);
	bool isFeasible(Z3Minimal &zHelper, PartialOrderWrapper &other, InstNum interfInst, InstNum curInst);

	void getLasts(unordered_set<InstNum> &lasts);
	PartialOrder& getPO() const;

	virtual bool operator== (const PartialOrderWrapper &other) const;
	// virtual bool operator<  (const PartialOrderWrapper &other) const;
	virtual void operator= (const PartialOrderWrapper &other);

	// void copy (const PartialOrderWrapper &copyFrom);
	void printAllPO() const;

	string toString() const;
};

namespace std {
template<>
struct hash<PartialOrderWrapper> {
	size_t operator() (const PartialOrderWrapper &po) const {
		// return (hash<unsigned short>()(in.getTid()) ^ 
		return hash<PartialOrder>()(po.getPO());
	}
};
}

#endif