#ifndef __INTERF__
#define __INTERF__

#include "common.h"
#include <forward_list>

// Node of an interf
class InterfNode {
public:
    InterfNode() {}
    InterfNode(llvm::Instruction *ld, llvm::Instruction *st) : load(ld), store(st) {}
    
    forward_list<InterfNode*>::iterator begin() {
        return child_list.begin();
    }

    forward_list<InterfNode*>::iterator end() {
        return child_list.end();
    }

    llvm::Instruction* getLoadInst() {
        return load;
    }

    llvm::Instruction* getStoreInst() {
        return store;
    }

private:
    llvm::Instruction *load;
    llvm::Instruction *store;
    forward_list<InterfNode*> child_list;
};

#endif
