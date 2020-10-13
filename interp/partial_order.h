#ifndef _PO
#define _PO

#include "common.h"
#include <functional>
// #include "z3_handler.h"

extern llvm::cl::opt<PrecisionLevel> Precision;

class PartialOrder {
	friend class PartialOrderWrapper;
	// all the instructions ordered after a in partial order are given 
	// by order[a]. In other words, Va.Vb (a,b) \in po => (b in order[a]).
	// If an instruction inst is not ordered with respect to any 
	// instruction in partial order so far, order[inst] will be empty 
	// set and ~E x. inst \in order[x]
	unordered_map<InstNum, unordered_set<InstNum>> order;
	unordered_set<InstNum> rmws;
	bool deleteOlder;

	PartialOrder(bool delOlder) :
		order(unordered_map<InstNum, unordered_set<InstNum>>()), 
		rmws(unordered_set<InstNum>()), deleteOlder(delOlder) {order.clear(); 
		// fprintf(stderr, "PO cons called %p\n",this);
		}
	// ~PartialOrder() = default;
	PartialOrder(const PartialOrder &po) : 
		order(po.order), rmws(po.rmws), 
		deleteOlder(po.deleteOlder) {
			// fprintf(stderr, "PO cons called %p\n",this);
		};

	// update ordering relation to get transitivity
	void makeTransitiveOrdering(const InstNum &from, 
		const InstNum &to, 
		std::unordered_map<InstNum, std::unordered_set<InstNum>>::iterator toItr);

	// adds an instruction with nothing ordered after it ib the order,
	// while deleting the older instructions from the same thread
	void addInst(const InstNum &inst);

	// Adds (from, to) to order if not already. 
	// Since partial order can't be cyclic, if (to, from) are already
	// in order, returns false. Else add (from, to) and returns true
	void addOrder(const InstNum &from, const InstNum &to);

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
	
	bool isDeletableInst (const InstNum &inst) const;
	bool isConsRMWP2(const PartialOrder & other) const;
	bool isConsRMWP3(const PartialOrder & other) const;
	
public:
	// PartialOrder() :
	// 	order(unordered_map<InstNum, unordered_set<InstNum>>()), 
	// 	rmws(unordered_set<InstNum>()), deleteOlder(true) {order.clear(); 
	// 	// fprintf(stderr, "PO cons called\n");
	// 	}

	// checks if the partial order other is consistent with this partial order
	// Two parial orders are consistent only if Va.Vb (a,b) \in order
	// (b,a) \notin other.order, or wiseversa
	bool isConsistent(const PartialOrder &other) const;
	bool isConsistentRMW(const PartialOrder &other) const;
	// domain-level feasibility checking
	bool isFeasible(const PartialOrder &other, InstNum &interfInst, InstNum &curInst) const;
	// get the last instructions in Partial Order
	// inst \in lasts iff nEa (inst, a) \in order
	void getLasts(unordered_set<InstNum> &lasts) const;
	// checks if (inst1, inst2) \in order
	bool isOrderedBefore(const InstNum &inst1, const InstNum &inst2) const;
	// 
	bool lessThan(const PartialOrder &other) const;
	bool getDeleteOlder() const {return deleteOlder;}


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
	size_t operator() (const PartialOrder *po) {
		return hash<string>() (po->toString());
		// size_t curhash = hash<bool>{}(po->getDeleteOlder());
		// // auto it = po->begin();
		// // if (it == po->end())
		// // 	return hash<InstNum>()(InstNum(0,0));
		// // size_t curhash = hash<InstNum>()(it->first);
		// // it++;
		// for (auto it=po->begin(); it!=po->end(); it++) {
		// 	curhash = curhash ^ hash<InstNum>()(it->first);
		// }
		// // fprintf(stderr, "returning hash %lu\n",curhash);
		// return curhash;
	}
};
}

struct hashPOPointer {
	size_t operator() (const PartialOrder *po) const {
		return hash<string>() (po->toString());
		// size_t curhash = hash<bool>{}(po->getDeleteOlder());
		// // auto it = po->begin();
		// // if (it == po->end())
		// // 	return hash<InstNum>()(InstNum(0,0));
		// // size_t curhash = hash<InstNum>()(it->first);
		// // it++;
		// for (auto it=po->begin(); it!=po->end(); it++) {
		// 	curhash = curhash ^ hash<InstNum>()(it->first);
		// }
		// // fprintf(stderr, "returning hash %lu\n",curhash);
		// return curhash;
	}
};

struct comparePOPointer {
	bool operator() (const PartialOrder* p1, const PartialOrder* p2) const {
		return *p1 == *p2;
	}
};

struct hashPOPair {
	size_t operator() (pair<const PartialOrder*, const PartialOrder*> poPair) const {
		string poref = "" + to_string((long int)poPair.first) + to_string((long int)poPair.second);
		return hash<string>()(poref);
	}
};

struct comparePOPair {
	bool operator() (
		pair<const PartialOrder*, const PartialOrder*> pair1,
		pair<const PartialOrder*, const PartialOrder*> pair2
	) const {
		return (*(pair1.first)==*(pair2.first) && 
				*(pair1.second)==*(pair2.second));
	}
};

struct hashPOInstNumPair {
	size_t operator() (
		pair<const PartialOrder*, const InstNum*> poInstNumPair
	) const {
 		string str = "" + 
			to_string((long int)poInstNumPair.first) + 
			poInstNumPair.second->toString();
 		return hash<string>() (str);
	}
};

struct comparePOInstNumPair {
	bool operator() (
		pair<const PartialOrder*, const InstNum*> pair1, 
		pair<const PartialOrder*, const InstNum*> pair2
	) const {
		return (*(pair1.first)==*(pair2.first) &&
				pair1.second==pair2.second);
	}
};

// namespace std{
// template<> 
// struct hash<pair<const PartialOrder*, const PartialOrder*>> {
// 	size_t operator () (pair<const PartialOrder*, const PartialOrder*> poPair) const {
// 		string poref = "" + to_string((long int)poPair.first) + to_string((long int)poPair.second);
// 		// fprintf(stderr, "poref: %s for pointers %ld and %ld", poref.c_str(), poPair.first, poPair.second);
// 		return hash<string>()(poref);
// 	}
// };

// template<>
// struct hash<pair<const PartialOrder*, const InstNum*>> {
// 	size_t operator () (pair<const PartialOrder*, const InstNum*> poInstNumPair) const {
// 		string str = "" + to_string((long int)poInstNumPair.first) + poInstNumPair.second->toString();
// 		return hash<string>() (str);
// 	}
// };


class PartialOrderWrapper {	
	static unordered_map<
		pair<const PartialOrder*, const PartialOrder*>, 
		const PartialOrder*,
		hashPOPair,
		comparePOPair> cachedJoin;

	static unordered_map<
		pair<const PartialOrder*, const PartialOrder*>, 
		const PartialOrder*,
		hashPOPair,
		comparePOPair> cachedMeet;

	static unordered_map<
		pair<const PartialOrder*, const InstNum*>, 
		const PartialOrder*,
		hashPOInstNumPair,
		comparePOInstNumPair> cachedAppend;
	
	// static void printAppendCache () {
	// 	for (auto it=cachedAppend.begin(); it!=cachedAppend.end(); it++) {
	// 		fprintf(stderr, "(%p,%p)::::%p\n",it->first.first, it->first.second, it->second);
	// 		fprintf(stderr, "(%s,%s)::::%s\n",it->first.first->toString().c_str(), it->first.second->toString().c_str(), it->second->toString().c_str());
	// 	}
	// }

public:
	static unordered_set<
		PartialOrder*, 
		hashPOPointer, 
		comparePOPointer> allPO;
	static const PartialOrder& addToSet(PartialOrder *po, bool &isAlreadyExist);

	// returns pair <true, ref of PO> if found, <false, null> if not found
	// pair<bool, PartialOrder&> find(PartialOrder *po);
	static bool hasInstance(PartialOrder &po);
	
	static PartialOrder& getEmptyPartialOrder(bool delOlder=true);
	static PartialOrder& append(const PartialOrder &curPO, InstNum &inst);
	static PartialOrder& addOrder(const PartialOrder &curPO, InstNum &from, InstNum &to);
	static PartialOrder& join(const PartialOrder &curPO, const PartialOrder &other);
	// compute meet of two partial orders i.e. leave only common 
	// instruction and common ordered pair
	static PartialOrder& meet(const PartialOrder &curPO, const PartialOrder &other);
	// static PartialOrder& remove(const PartialOrder &curPO, InstNum &inst);

	static void printAllPO();
	static void clearAllPO();
};

#endif
