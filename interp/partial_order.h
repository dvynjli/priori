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
	map<InstNum, set<InstNum>> order;
	set<InstNum> rmws;

	// update ordering relation to get transitivity
	bool makeTransitiveOrdering(InstNum from, 
		InstNum to, 
		std::map<InstNum, std::set<InstNum>>::iterator toItr);

	// adds an instruction with nothing ordered after it ib the order,
	// while deleting the older instructions from the same thread
	bool addInst(Z3Minimal &zHelper, InstNum inst);
	bool isRMWInst(InstNum inst);

public:
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
	virtual bool operator<  (const PartialOrder &other) const;

	map<InstNum, set<InstNum>>::const_iterator begin() const;
	map<InstNum, set<InstNum>>::const_iterator end() const;

	void copy (const PartialOrder &copyFrom);

	string toString();
};

#endif