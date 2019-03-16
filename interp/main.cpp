#include "common.h"
#include "domain.h"
#include "analyzer.h"
#include "z3_handler.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Operator.h"

class VerifierPass : public ModulePass {

    /* TODO: Should I create threads as a set or vector??
        Vector- interferences of t1 with t1 will be explored in case more than one thread of same func are present
        Set and interference of each thread with itself are also explored, support to inf threads of same func can be added 
    */
    vector<Function*> threads;
    map<Function*, map<Instruction*, Domain>> programState;
    map<Function*, Domain> funcInitDomain;
    map<Function*, vector< vector< map<Instruction*, Instruction*>>>> feasibleInterfences;
    map <string, Value*> nameToValue;
    map <Value*, string> valueToName;

    
    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        // Domain initDomain = Domain();
        // TODO: get domain type based on comman line arguments
        string domainType = "box";
        
        initThreadDetails(M, getGlobalIntVars(M), domainType);

        // analyzeProgram(M);

        // unsat_core_example1();
    }

    vector<string> getGlobalIntVars(Module &M) {
        vector<string> intVars;
        for (auto it = M.global_begin(); it != M.global_end(); it++){
            // cerr << "Global var: " << it->getName() << endl;
            // fprintf(stderr, "Global var: %s of type: %d\n", it->getName(), it->getValueType()->getTypeID());
            // it->getValueType()->print(errs());
            if (it->getValueType()->isIntegerTy()) {
                string varName = it->getName();
                intVars.push_back(varName);
                Value * varInst = &(*it);
                nameToValue.emplace(varName, varInst);
                valueToName.emplace(varInst, varName);
            }
            else if (StructType* structTy = dyn_cast<StructType>(it->getValueType())) {
                if  (!structTy->getName().compare("struct.std::atomic")) {
                    string varName = it->getName();
                    intVars.push_back(varName);
                    Value * varInst = &(*it);
                    nameToValue.emplace(varName, varInst);
                    valueToName.emplace(varInst, varName);
                }
                else {
                    fprintf(stderr, "WARNING: found global structure: %s. It will not be analyzed", structTy->getName());
                }
            }

            // Needed for locks
            // if (it->getValueType()->isStructTy()){
            //     fprintf(stderr, "Struct: %s of type", it->getName());
            //     // if (auto st = dyn_cast<StructType>(it->getValueType())){
            //     //     fprintf(stderr, "cast\n");
            //     // }
            //     fprintf(stderr, "%s\n", it->getValueType()->getStructName());
            // }

        }
        fprintf(stderr, "DEBUG: Total global var = %lu\n", intVars.size());
        return intVars;
    }

    Function* getMainFunction(Module &M){
        Function *mainF;
        
        for(auto funcItr = M.begin(); funcItr != M.end(); funcItr++) {
            if (funcItr->getName() == "main"){
                mainF = &(*funcItr);
                break;    
            }
        }
        return mainF;
    }

    map<Function*, map<string, vector<pair<Instruction*, Instruction*>>>> makeAllLSPair (
            map<Function*, map<string, unordered_set<Instruction*>>> allLoads,
            map<Function*, map<string, unordered_set<Instruction*>>> allStores
        ) {
        map<Function*, map<string, vector<pair<Instruction*, Instruction*>>>> allLoadStorePair;

        for (auto funcItr=allLoads.begin(); funcItr!=allLoads.end(); ++funcItr){
        Function * curFunc = funcItr->first;
        auto curFuncLoads = funcItr->second;
        // loads of each variable
        for (auto varItr=curFuncLoads.begin(); varItr!=curFuncLoads.end(); ++varItr) {
            string varName = varItr->first;
            auto loadsOfVar = varItr->second;
            // iterate over stores
            for (auto storeFunItr=allStores.begin(); storeFunItr!=allStores.end(); ++storeFunItr) {
                Function *storeFunc = storeFunItr->first;
                // ne need to check interfernce of a function with itself
                if (storeFunc != curFunc) {
                    auto stores = storeFunItr->second;
                    // if a store of current variable exist
                    auto searchVarInStores = stores.find(varName);
                    if (searchVarInStores != stores.end()) {
                        auto storesOfVar = searchVarInStores->second;
                        // for all loads of this variable in curFunc
                        for (auto loadsOfVarItr=loadsOfVar.begin(); loadsOfVarItr!=loadsOfVar.end(); ++loadsOfVarItr) {
                            Instruction* currenLoad = *loadsOfVarItr;
                            // for all stores of this variable from all the other functions
                            for (auto storesOfVarItr=storesOfVar.begin(); storesOfVarItr!=storesOfVar.end(); ++storesOfVarItr) {
                                Instruction* currentStore = *storesOfVarItr;
                                pair<Instruction*, Instruction*> loadStorePair = make_pair(currenLoad, currentStore);
                                // store (l,s) pair in
                                allLoadStorePair[curFunc][varName].push_back(loadStorePair);
                                /* // print the pair created
                                errs() << "current load: ";
                                loadStorePair.first->print(errs());
                                errs() << "\ncurrent store: ";
                                loadStorePair.second->print(errs());
                                errs() << "\n"; */
                            }
                        }
                    }
                }
            }
        }
    }
        /* 
        // print the allLoadStorePair
        for (auto it1=allLoadStorePair.begin(); it1!=allLoadStorePair.end(); ++it1) {
            errs() << "Load store pair of func " << it1->first->getName() << ":\n";
            map<string, vector<pair<Instruction*, Instruction*>>> lsPairOfCurFunc = it1->second;
            for (auto it2=lsPairOfCurFunc.begin(); it2!=lsPairOfCurFunc.end(); ++it2) {
                errs() <<"\tVar: " << it2->first <<"\n";
                vector<pair<Instruction*, Instruction*>> lsPairOfVar = it2->second;
                for (auto it3=lsPairOfVar.begin(); it3!=lsPairOfVar.end(); ++it3) {
                    pair<Instruction*, Instruction*> lsPair = *it3;
                    errs() << "\t\tLoad: ";
                    lsPair.first->print(errs());
                    errs() << "\n\t\tStore: ";
                    lsPair.second->print(errs());
                    errs() << "\n";
                }
            }
        } */
        return allLoadStorePair;
    }

    map<Function*, vector< map<Instruction*, Instruction*>>> makeInterfsFromLSPair (
        map<Function*, map<string, vector<pair<Instruction*, Instruction*>>>> allLoadStorePair
        ){
        map<Function*, vector< map<Instruction*, Instruction*>>> interfs;
        // for (auto allLSPairItr = allLoadStorePair.begin(); allLSPairItr!=allLoadStorePair.end(); ++allLSPairItr) {
        //     Function *curFunc = allLSPairItr->first;
        //     auto allLSPairOfFun = allLSPairItr->second;
        //     for (auto varToLSPairItr=allLSPairOfFun.begin(); varToLSPairItr!=allLSPairOfFun.end(); ++varToLSPairItr) {
        //         string varName = varToLSPairItr->first;
        //         auto LSPairOfVar = varToLSPairItr->second;
        //     }
        // }
        return interfs;
    }

    map<Function*, map<Instruction*, vector<Instruction*>>> getLoadsToAllStoresMap (
        map<Function*, unordered_map<Instruction*, string>> allLoads,
        map<Function*, map<string, unordered_set<Instruction*>>> allStores
        ){
        map<Function*, map<Instruction*, vector<Instruction*>>> loadsToAllStores;

        for (auto allLoadsItr=allLoads.begin(); allLoadsItr!=allLoads.end(); ++allLoadsItr) {
            Function* curFunc = allLoadsItr->first;
            auto curFuncLoads = allLoadsItr->second;
            for (auto curFuncLoadsItr=curFuncLoads.begin(); curFuncLoadsItr!=curFuncLoads.end(); ++curFuncLoadsItr) {
                Instruction *load =curFuncLoadsItr->first;
                string loadVar = curFuncLoadsItr->second;
                vector<Instruction*> allStoresForCurLoad;
                // loads of same var from all other functions
                for (auto allStoresItr=allStores.begin(); allStoresItr!=allStores.end(); ++allStoresItr) {
                    Function *otherFunc = allStoresItr->first;
                    // need interfernce only from other threads
                    if (otherFunc != curFunc) {
                        auto otherFuncStores = allStoresItr->second;
                        auto searchStoresOfVar = otherFuncStores.find(loadVar);
                        if (searchStoresOfVar != otherFuncStores.end()) {
                            auto allStoresFromFun = searchStoresOfVar->second;
                            copy(allStoresFromFun.begin(), 
                                allStoresFromFun.end(), 
                                inserter(allStoresForCurLoad, 
                                allStoresForCurLoad.end()));
                        }
                    }
                }
                // loadsToAllStores[curFunc][load] = allStoresForCurLoad;
            }
        }

        return loadsToAllStores;
    }

    // void printLoadsToAllStores(map<Function*, map<Instruction*, vector<Instruction*>>> loadsToAllStores) {
    //     for (auto it1=loadsToAllStores.begin(); it1!=loadsToAllStores.end(); ++it1) {
    //         errs () << "***Function " << it1->first->getName() << ":\n";
    //         auto l2s = it1->second;
    //         for (auto it2=l2s.begin(); it2!=l2s.end(); ++it2) {
    //             errs() << "Stores for Load: ";
    //             it2->first->print(errs());
    //             errs() << "\n";
    //             auto stores = it2->second;
    //             for (auto it3=stores.begin(); it3!=stores.end(); ++it3) {
    //                 errs() << "\t";
    //                 (*it3)->print(errs());
    //                 errs() << "\n";
    //             }
    //         }
    //     }
    // }

    void getFeasibleInterferences (
        map<Function*, unordered_map<Instruction*, string>> allLoads,
        map<Function*, map<string, unordered_set<Instruction*>>> allStores
        ){
        map<Function*, vector< map<Instruction*, Instruction*>>> allInterfs;
        map<Function*, map<Instruction*, vector<Instruction*>>> loadsToAllStores;
        // Make all permutations
        // TODO: add dummy env i.e. load from itself
        loadsToAllStores = getLoadsToAllStoresMap(allLoads, allStores);

        // Check feasibility of permutations and save them in feasibleInterfences
    }

    void initThreadDetails(Module &M, vector<string> globalVars, string domainType) {
        map<Function*, unordered_map<Instruction*, string>> allLoads;
        map<Function*, map<string, unordered_set<Instruction*>>> allStores;

        //find main function
        Function *mainF = getMainFunction(M);

        threads.push_back(mainF);

        // TODO: Why queue? Is it better to take it as set
        queue<Function*> funcQ;
        unordered_set<Function*> funcSet;
        funcQ.push(mainF);
        funcSet.insert(mainF);
       
        int ssaVarCounter = 0;

        while(!funcQ.empty())
        {
            Function *func = funcQ.front();
            funcQ.pop();
            vector<string> funcVars(globalVars);
            for(auto block = func->begin(); block != func->end(); block++)          //iterator of Function class over BasicBlock
            {
                map<string, unordered_set<Instruction*>> varToStores;
                unordered_map<Instruction*, string> varToLoads;
                for(auto it = block->begin(); it != block->end(); it++)       //iterator of BasicBlock over Instruction
                {
                    if (CallInst *call = dyn_cast<CallInst>(it)) {
                        if(!call->getCalledFunction()->getName().compare("pthread_create")) {
                            if (Function* newThread = dyn_cast<Function> (call->getArgOperand(2)))
                            {  
                                auto inSet = funcSet.insert(newThread);
                                if (inSet.second) {
                                    funcQ.push(newThread);
                                    threads.push_back(newThread); 	
                                }
                            }
                            // TODO: need to add dominates rules
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
                        }
                        else {
                            cout << "unknown function call:\n";
                            it->print(errs());
                            errs() <<"\n";
                        }
                    }
                    else if (StoreInst *storeInst = dyn_cast<StoreInst>(it)) {
                        Value* destVar = storeInst->getPointerOperand();
                        if(GEPOperator *gepOp = dyn_cast<GEPOperator>(destVar)){
                            destVar = gepOp->getPointerOperand();
                        }
                        string destVarName = getNameFromValue(destVar);
                        if (dyn_cast<GlobalVariable>(destVar)) {
                            varToStores[destVarName].insert(storeInst);
                        }
                    }
                    else if (it->isTerminator()) {

                    }

                    else {
                        Instruction *inst = dyn_cast<Instruction>(it);
                        string varName = "var" + to_string(ssaVarCounter);
                        ssaVarCounter++;
                        nameToValue.emplace(varName, inst);
                        valueToName.emplace(inst, varName);
                        funcVars.push_back(varName);

                        if (LoadInst *loadInst = dyn_cast<LoadInst>(it)) {
                            Value* fromVar = loadInst->getOperand(0);
                            if(GEPOperator *gepOp = dyn_cast<GEPOperator>(fromVar)){
                                fromVar = gepOp->getPointerOperand();
                            }
                            string fromVarName = getNameFromValue(fromVar);
                            if (dyn_cast<GlobalVariable>(fromVar)) {
                                varToLoads.emplace(loadInst, fromVar);
                            }
                        }
                    }
                }

                // Save loads stores function wise
                allStores.emplace(func, varToStores);
                allLoads.emplace(func, varToLoads);

            }
            Domain curFuncDomain;
            curFuncDomain.init(domainType, funcVars);
            funcInitDomain.emplace(func, curFuncDomain);
        }
        getFeasibleInterferences(allLoads, allStores);
    }

    void testApplyInterf() {
        // Sample code to test applyInterference()
        auto funIt = funcInitDomain.begin();
        Function *fun1 = funIt->first;
        Domain fun1Domain = funIt->second;
        fun1Domain.performUnaryOp(STORE, "x", 1);
        funIt++;
        Function *fun2 = funIt->first;
        Domain fun2Domain = funIt->second;
        fun2Domain.applyInterference("x", fun1Domain);
    }

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs

        map<Function*, map<Instruction*, Instruction*>> feasibleInterf;

        for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
            Function *curFunc = (*funcItr);
            fprintf(stderr, "\n******DEBUG: Analyzing thread %s*****\n", curFunc->getName());

            // find feasible interfernce for current function
            map<Instruction*, Instruction*> *curFuncInterf;

            auto searchInterf = feasibleInterf.find(curFunc);
            if (searchInterf != feasibleInterf.end()) {
                curFuncInterf = &(searchInterf->second);
            }
            
            map<Instruction*, Domain> newFuncDomain;

            // all stores and loads to a variable from one thread
            // Need to compute it only ones unless we are stopping analysis in the middle if domain is bottom

            // TODO: run this function for all interfs in curFuncInters
            newFuncDomain = analyzeThread(*funcItr, curFuncInterf);
            // join newFuncDomain of all feasibleInterfs and replace old one in state
            auto searchFunDomain = programState.find(curFunc);
            if (searchFunDomain == programState.end()) {
                programState.emplace(curFunc, newFuncDomain);
            }
            else {
                programState.emplace(curFunc, joinDomainByInstruction(searchFunDomain->second, newFuncDomain));
            }

            // curFuncInterf->clear();
        }
    }

    map<Instruction*, Domain> analyzeThread (Function *F, map<Instruction*, Instruction*> *interf) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        map <Instruction*, Domain> curFuncDomain;
        // Domain predDomain;

        for(auto bbItr=F->begin(); bbItr!=F->end(); ++bbItr){
            BasicBlock *currentBB = &(*bbItr);

            Domain predDomain = funcInitDomain[F];
            // predDomain.printDomain();
            // initial domain of pred of cur bb to join of all it's pred
            for (BasicBlock *Pred : predecessors(currentBB)){
                auto searchPredBBDomain = curFuncDomain.find(Pred->getTerminator());
                if (searchPredBBDomain != curFuncDomain.end())
                    predDomain.joinDomain(searchPredBBDomain->second);
                // TODO: if the domain is empty? It means pred bb has not been analyzed so far
                // we can't analyze current BB
            }
            // if termination statement
                // if coditional branching
                // if unconditional branching

            predDomain = analyzeBasicBlock(currentBB, predDomain, curFuncDomain);
        }

        return curFuncDomain;
    }

    Domain analyzeBasicBlock (BasicBlock *B, Domain curDomain, map <Instruction*, Domain> curFuncDomain) {
        // check type of inst, and performTrasformations
        
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);

            errs() << "DEBUG: Analyzing: ";
            currentInst->print(errs());
            errs()<<"\n";

            if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(currentInst)) {
                // auto searchName = valueToName.find(currentInst);
                // if (searchName == valueToName.end()) {
                //     fprintf(stderr, "ERROR: new mem alloca");
                //     allocaInst->dump();
                //     exit(0);
                // }
            }
            else if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(instItr)) {
                curDomain = checkBinInstruction(binOp, curDomain);
            }
            else if (StoreInst *storeInst = dyn_cast<StoreInst>(instItr)) {
                curDomain = checkStoreInst(storeInst, curDomain);
            }
            else if (UnaryInstruction *unaryInst = dyn_cast<UnaryInstruction>(instItr)) {
                curDomain = checkUnaryInst(unaryInst, curDomain);
            }
            // RMW, CMPXCHG
            curFuncDomain.emplace(currentInst, curDomain);
        }
        
        
        return curDomain;
    }

    string getNameFromValue(Value *val) {
        if(GEPOperator *gepOp = dyn_cast<GEPOperator>(val)){
           val = gepOp->getPointerOperand();
        }
        auto searchName = valueToName.find(val);
        if (searchName == valueToName.end()) {
            errs() << "ERROR: Instrution not found in Instruction to Name map\n";
            val->print(errs());
            errs()<<"\n";
            // fprintf(stderr, "ERROR: Instrution not found in Instruction to Name map\n");
            exit(0);
        }
        return searchName->second;
    }

    //  call approprproate function for the inst passed
    Domain checkBinInstruction(BinaryOperator* binOp, Domain curDomain) {
        operation oper;
        switch (binOp->getOpcode()) {
            case Instruction::Add:
                oper = ADD;
                break;
            case Instruction::Sub:
                oper = SUB;
                break;
            case Instruction::Mul:
                oper = MUL;
                break;
            // TODO: add more cases
            default:
                fprintf(stderr, "WARNING: unknown operation: ");
                binOp->print(errs());
                return curDomain;
        }

        string destVarName = getNameFromValue(binOp);
        Value* fromVar1 = binOp->getOperand(0);
        Value* fromVar2 = binOp->getOperand(1);
        if (ConstantInt *constFromVar1 = dyn_cast<ConstantInt>(fromVar1)) {
            int constFromIntVar1= constFromVar1->getValue().getSExtValue();
            if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
                int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
                curDomain.performBinaryOp(oper, destVarName, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                curDomain.performBinaryOp(oper, destVarName, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            curDomain.performBinaryOp(oper, destVarName, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            curDomain.performBinaryOp(oper, destVarName, fromVar1Name, fromVar2Name);
        }

        return curDomain;
    }

    Domain checkStoreInst(StoreInst* storeInst, Domain curDomain) {
        Value* destVar = storeInst->getPointerOperand();
        string destVarName = getNameFromValue(destVar);

        Value* fromVar = storeInst->getValueOperand();
        
        if (ConstantInt *constFromVar = dyn_cast<ConstantInt>(fromVar)) {
            int constFromIntVar = constFromVar->getValue().getSExtValue();
            curDomain.performUnaryOp(STORE, destVarName, constFromIntVar);
        }
        else if (Argument *argFromVar = dyn_cast<Argument>(fromVar)) {
            // TODO: handle function arguments

        }
        else {
            string fromVarName = getNameFromValue(fromVar);
            curDomain.performUnaryOp(STORE, destVarName, fromVarName);
        }

        return curDomain;
    }

    Domain checkUnaryInst(UnaryInstruction* unaryInst, Domain curDomain) {
        Value* fromVar = unaryInst->getOperand(0);
        string fromVarName = getNameFromValue(fromVar);
        string destVarName = getNameFromValue(unaryInst);
        
        operation oper;
        switch (unaryInst->getOpcode()) {
            case Instruction::Load:
                oper = LOAD;
                break;
            // TODO: add more cases
            default: 
                fprintf(stderr, "ERROR: unknown operation: ");
                unaryInst->print(errs());
                return curDomain;
        }
        
        curDomain.performUnaryOp(oper, destVarName, fromVarName);

        return curDomain;
    }

    map<Instruction*, Domain> joinDomainByInstruction(
            map<Instruction*, Domain> map1,
            map<Instruction*, Domain> map2
            ) {
        // for (auto it = map1.)
    }

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true /*change it to true for analysis pass*/);
