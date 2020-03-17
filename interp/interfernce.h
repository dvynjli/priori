#ifndef __INTERF__
#define __INTERF__

#include "common.h"
#include <forward_list>

/// Node of an interf
extern set<pair<llvm::Instruction*, llvm::Instruction*>> allLSPairs;

class InterfNode {
public:
    InterfNode() {}
    InterfNode(pair<llvm::Instruction*, llvm::Instruction*> *ls) : lsPair(ls) {}
    InterfNode(llvm::Instruction *ld, llvm::Instruction *st) {
        auto lsPairPtr = allLSPairs.insert(make_pair(ld, st));
        lsPair = &(*lsPairPtr.first);
    }
    
    forward_list<InterfNode*>::iterator begin() {
        return child_list.begin();
    }

    forward_list<InterfNode*>::iterator end() {
        return child_list.end();
    }

    forward_list<InterfNode*>::iterator before_begin() {
        return child_list.before_begin();
    }

    // InterfNode *getLastChild() {
    //     return child_list
    // }

    llvm::Instruction* getLoadInst() {
        return lsPair->first;
    }

    llvm::Instruction* getStoreInst() {
        return lsPair->second;
    }

    forward_list<InterfNode*>* getChildList() {
        return &child_list;
    }

    int getNumChild() {
        int i = 0;
        for (auto it: child_list) {
            i++;
        }
        return i;
    }

private:
    // llvm::Instruction *load;
    // llvm::Instruction *store;
    const pair<llvm::Instruction*, llvm::Instruction*> *lsPair;
    forward_list<InterfNode*> child_list;
   
};

#endif
