#ifndef _PO
#define _PO

#include "common.h"
// #include "z3_handler.h"

class PartialOrder {
	friend class PartialOrderWrapper;
	// all the instructions ordered after a in partial order are given 
	// by order[a]. In other words, Va.Vb (a,b) \in po => (b in order[a]).
	// If an instruction inst is not ordered with respect to any 
	// instruction in partial order so far, order[inst] will be empty 
	// set and ~E x. inst \in order[x]
	unordered_map<InstNum, unordered_set<InstNum>> order;
	unordered_set<InstNum> rmws;

	// update ordering relation to get transitivity
	bool makeTransitiveOrdering(const InstNum &from, 
		const InstNum &to, 
		std::unordered_map<InstNum, std::unordered_set<InstNum>>::iterator toItr);

	// adds an instruction with nothing ordered after it ib the order,
	// while deleting the older instructions from the same thread
	bool addInst(const InstNum &inst);
	bool isRMWInst(const InstNum &inst);

	// Adds (from, to) to order if not already. 
	// Since partial order can't be cyclic, if (to, from) are already
	// in order, returns false. Else add (from, to) and returns true
	bool addOrder(const InstNum &from, const InstNum &to);

	// Adds inst such that Va \in order, (a, inst) \in order.
	// Returns false if inst \in order and order[inst] != {}
	void append(const InstNum &inst);

	// Joins two partial orders maintaing the ordering relation in both
	// If this is not possible (i.e. joining will result in cycle), 
	// returns false
	void join(const PartialOrder &other);

	// checks if inst is a part of this partial order
	bool isExists(const InstNum &inst) const;

	// Removes inst from the element of the set. It should also remove
	// all (x, inst) and (inst, x) pair from order for all possible 
	// values of x
	void remove(const InstNum &inst);
	
public:
	PartialOrder() :
		order(unordered_map<InstNum, unordered_set<InstNum>>()), 
		rmws(unordered_set<InstNum>()) {order.clear(); 
		// fprintf(stderr, "PO cons called\n");
		}
	// ~PartialOrder() = default;
	PartialOrder(const PartialOrder &po) : order(po.order), rmws(po.rmws) {};

	// checks if the partial order other is consistent with this partial order
	// Two parial orders are consistent only if Va.Vb (a,b) \in order
	// (b,a) \notin other.order, or wiseversa
	bool isConsistent(const PartialOrder &other);
	bool isConsistentRMW(const PartialOrder &other);
	// domain-level feasibility checking
	bool isFeasible(const PartialOrder &other, InstNum &interfInst, InstNum &curInst);
	// get the last instructions in Partial Order
	// inst \in lasts iff nEa (inst, a) \in order
	void getLasts(unordered_set<InstNum> &lasts) const;
	// checks if (inst1, inst2) \in order
	bool isOrderedBefore(const InstNum &inst1, const InstNum &inst2) const;

	unordered_map<InstNum, unordered_set<InstNum>>::const_iterator begin() const;
	unordered_map<InstNum, unordered_set<InstNum>>::const_iterator end() const;
	virtual bool operator== (const PartialOrder &other) const;
	// virtual void operator= (const PartialOrder &other);
	// virtual bool operator<  (const PartialOrder &other) const;
	void copy (const PartialOrder &copyFrom);
	void clear();
	string toString() const;
};

namespace std{
template<>
struct hash<PartialOrder*> {
	size_t operator() (const PartialOrder *po) const {
		// return (hash<unsigned short>()(in.getTid()) ^ 
		auto it = po->begin();
		if (it == po->end())
			return hash<InstNum>()(InstNum(0,0));
		size_t curhash = hash<InstNum>()(it->first);
		it++;
		for (; it!=po->end(); it++) {
			curhash = curhash ^ hash<InstNum>()(it->first);
		}
		// fprintf(stderr, "returning hash\n");
		return curhash;
	}
};
}

class PartialOrderWrapper {	
public:
	static unordered_set<PartialOrder*> allPO;
	static const PartialOrder& addToSet(PartialOrder *po, bool &isAlreadyExist);

	// returns pair <true, ref of PO> if found, <false, null> if not found
	// pair<bool, PartialOrder&> find(PartialOrder *po);
	static bool hasInstance(PartialOrder &po);
	
	static PartialOrder append(const PartialOrder &curPO, InstNum &inst);
	static PartialOrder addOrder(PartialOrder &curPO, InstNum &from, InstNum &to);
	static PartialOrder join(PartialOrder &curPO, const PartialOrder &other);
	static PartialOrder remove(PartialOrder &curPO, InstNum &inst);

	static void printAllPO();
	static void clearAllPO();
};

#endif