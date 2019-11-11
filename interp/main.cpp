#include "common.h"
#include "domain.h"
#include "analyzer.h"
#include "z3_handler.h"


// Processing command line arguments

// command line argument for domain type
cl::opt<DomainTypes> AbsDomType(cl::desc("Choose abstract domain to be used"),
    cl::values(
        clEnumVal(interval , "use interval domain"),
        clEnumVal(octagon, "use octagon domain")));
cl::opt<bool> useZ3     ("z3", cl::desc("Enable interferce pruning using Z3"));
cl::opt<bool> noPrint   ("no-print", cl::desc("Enable interferce pruning using Z3"));
cl::opt<bool> minimalZ3 ("z3-minimal", cl::desc("Enable interferce pruning using Z3"));
cl::opt<bool> useMOHead ("useMOHead", cl::desc("Enable interference pruning using Z3 using modification order head based analysis"));
cl::opt<bool> useMOPO ("useMOPO", cl::desc("Enable interference pruning using Z3 using partial order over modification order based analysis"));

class VerifierPass : public ModulePass {

    /* TODO: Should I create threads as a set or vector??
        Vector- interferences of t1 with t1 will be explored in case more than one thread of same func are present
        Set and interference of each thread with itself are also explored, support to inf threads of same func can be added 
    */
    
    typedef EnvironmentPOMO Environment;

    vector <Function*> threads;
    map<StoreInst*, StoreInst*> prevRelWriteOfSameVar;
    unordered_map <Function*, unordered_map<Instruction*, Environment>> programState;
    unordered_map <Function*, Environment> funcInitEnv;
    unordered_map <Function*, vector< unordered_map<Instruction*, Instruction*>>> feasibleInterfences;
    unordered_map <string, Value*> nameToValue;
    unordered_map <Value*, string> valueToName;
    Z3Minimal zHelper;


    bool runOnModule (Module &M) {
        double start_time = omp_get_wtime();
        vector<string> globalVars = getGlobalIntVars(M);
        // zHelper.initZ3(globalVars);
        initThreadDetails(M, globalVars);
        // testPO();
        analyzeProgram(M);
        checkAssertions();
        double time = omp_get_wtime() - start_time;
        // testApplyInterf();
        // unsat_core_example1();
        if (!noPrint) { 
            errs() << "----DONE----\n";
        }
        fprintf(stderr, "Time elapsed: %f\n", time);
    }

    vector<string> getGlobalIntVars(Module &M) {
        vector<string> intVars;
        for (auto it = M.global_begin(); it != M.global_end(); it++){
            // errs() << "Global var: " << it->getName() << "\tType: ";
            // it->getValueType()->print(errs());
            // errs() << "\n";
            if (it->getValueType()->isIntegerTy() && it->getName()!="__dso_handle") {
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
                    // errs() << "WARNING: found global structure:" << structTy->getName() << ". It will not be analyzed\n";
                }
            }
            else {
                // errs() << "WARNING: It will not be analyzed\n";
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
        if (!noPrint)
            errs() << "DEBUG: Total global var = " << intVars.size() << "\n";
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

    unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> getLoadsToAllStoresMap (
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores
    ){
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores;

        for (auto allLoadsItr=allLoads.begin(); allLoadsItr!=allLoads.end(); ++allLoadsItr) {
            Function* curFunc = allLoadsItr->first;
            auto curFuncLoads = allLoadsItr->second;
            for (auto curFuncLoadsItr=curFuncLoads.begin(); curFuncLoadsItr!=curFuncLoads.end(); ++curFuncLoadsItr) {
                Instruction *load =curFuncLoadsItr->first;
                string loadVar = curFuncLoadsItr->second;
                // errs() << "coping loads of var " << loadVar << " of function " << curFunc->getName() << "\n";
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
                            // errs() << "#stores = " << allStoresFromFun.size() << "\n";
                            copy(allStoresFromFun.begin(), 
                                allStoresFromFun.end(), 
                                inserter(allStoresForCurLoad, 
                                allStoresForCurLoad.end()));
                        }
                    }
                }
                // Push the current context to read from self envionment
                allStoresForCurLoad.push_back(nullptr);
                loadsToAllStores[curFunc][load] = allStoresForCurLoad;
            }
        }
        // printLoadsToAllStores(loadsToAllStores);

        return loadsToAllStores;
    }

    void initThreadDetails(Module &M, vector<string> globalVars) {
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads;
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores;
        vector< pair <string, pair<Instruction*, Instruction*>>> relations;

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
            vector<string> funcVars;
            
            // errs() << "----analyzing funtion: " << func->getName() << "\n";
            unordered_map<string, unordered_set<Instruction*>> varToStores;
            unordered_map<Instruction*, string> varToLoads;
                
            for(auto block = func->begin(); block != func->end(); block++)          //iterator of Function class over BasicBlock
            {
                Instruction *lastGlobalInst=nullptr;
                map<string, Instruction*> lastGlobalOfVar;
                map<string, StoreInst*> lastRelWrite;
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
                                    // need to add dominates rules
                                    Instruction *lastGlobalInstBeforeCall = getLastGlobalInst(call);
                                    Instruction *nextGlobalInstAfterCall  = getNextGlobalInst(call->getNextNode());
                                    Instruction *firstGlobalInstInCalled  = getNextGlobalInst(&*(newThread->begin()->begin()));
                                    // lastGlobalInstBeforeCall (or firstGlobalInstInCalled) == nullptr means there 
                                    // no global instr before thread create in current function (or in newly created thread)
                                    if (lastGlobalInstBeforeCall) {
                                        if (minimalZ3) zHelper.addSB(lastGlobalInstBeforeCall, call);
                                        relations.push_back(make_pair("mhb", make_pair(lastGlobalInstBeforeCall, call)));
                                    }
                                    if (nextGlobalInstAfterCall) {
                                        if (minimalZ3) zHelper.addSB(call, nextGlobalInstAfterCall);
                                        relations.push_back(make_pair("mhb", make_pair(call, nextGlobalInstAfterCall)));
                                    }
                                    if (firstGlobalInstInCalled) {
                                        if (minimalZ3) zHelper.addSB(call, firstGlobalInstInCalled);
                                        relations.push_back(make_pair("mhb", make_pair(call, firstGlobalInstInCalled)));
                                    }
                                    if (lastGlobalInst) {
                                        if (minimalZ3) zHelper.addSB(lastGlobalInst, call);
                                        relations.push_back(make_pair("po", make_pair(lastGlobalInst, call)));
                                    }
                                }

                            }
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
                            Instruction *lastGlobalInstBeforeCall = getLastGlobalInst(call);
                            Instruction *nextGlobalInstAfterCall  = getNextGlobalInst(call->getNextNode());
                            vector<Instruction*> lastGlobalInstInCalled = getLastInstOfPthreadJoin(call);
                            if (nextGlobalInstAfterCall) {
                                if (minimalZ3) zHelper.addSB(call, nextGlobalInstAfterCall);
                                relations.push_back(make_pair("mhb", make_pair(call, nextGlobalInstAfterCall)));
                            }
                            if (lastGlobalInstBeforeCall) {
                                if (minimalZ3) zHelper.addSB(lastGlobalInstBeforeCall, call);
                                relations.push_back(make_pair("mhb", make_pair(lastGlobalInstBeforeCall, call)));
                            }
                            for (auto it=lastGlobalInstInCalled.begin(); it!=lastGlobalInstInCalled.end(); ++it) {
                                if (minimalZ3) zHelper.addSB(*it, call);
                                relations.push_back(make_pair("mhb", make_pair(*it, call)));
                            }
                            if (lastGlobalInst) {
                                if (minimalZ3) zHelper.addSB(lastGlobalInst, call);
                                relations.push_back(make_pair("po", make_pair(lastGlobalInst, call)));
                            }
                        }
                        else {
                            if (!noPrint) {
                                errs() << "unknown function call:\n";
                                it->print(errs());
                                errs() <<"\n";
                            }
                        }
                    }
                    else if (StoreInst *storeInst = dyn_cast<StoreInst>(it)) {
                        Value* destVar = storeInst->getPointerOperand();
                        if(GEPOperator *gepOp = dyn_cast<GEPOperator>(destVar)){
                            destVar = gepOp->getPointerOperand();
                        }
                        if (dyn_cast<GlobalVariable>(destVar)) {
                            string destVarName = getNameFromValue(destVar);
                            varToStores[destVarName].insert(storeInst);
                            if (storeInst->getOrdering() == llvm::AtomicOrdering::Release ||
                                storeInst->getOrdering() == llvm::AtomicOrdering::AcquireRelease ||
                                storeInst->getOrdering() == llvm::AtomicOrdering::SequentiallyConsistent) {
                                lastRelWrite[destVarName] = storeInst;
                            }
                            else if (lastRelWrite[destVarName]) {
                                prevRelWriteOfSameVar[storeInst] = lastRelWrite[destVarName];
                            }
                            // errs() << "****adding store instr for: ";
                            // printValue(storeInst);
                            // zHelper.addStoreInstr(storeInst);
                            if (lastGlobalInst) {
                                if (minimalZ3) zHelper.addSB(lastGlobalInst, storeInst);
                                relations.push_back(make_pair("po", make_pair(lastGlobalInst, storeInst)));
                            } 
                            // no global operation yet. Add MHB with init
                            else {
                                relations.push_back(make_pair("mhb", make_pair(lastGlobalInst, storeInst)));
                            }
                            if (lastGlobalOfVar[destVarName])
                                relations.push_back(make_pair("mhb", make_pair(lastGlobalOfVar[destVarName], storeInst)));
                            lastGlobalOfVar[destVarName] = storeInst;
                            lastGlobalInst = storeInst;
                        }
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
                                varToLoads[loadInst]=fromVarName;
                                // errs() << "****adding load instr for: ";
                                // printValue(loadInst);
                                // zHelper.addLoadInstr(loadInst);
                                if (lastGlobalInst) {
                                    if (minimalZ3) zHelper.addSB(lastGlobalInst, loadInst);
                                    relations.push_back(make_pair("po", make_pair(lastGlobalInst, loadInst)));
                                }
                                // no global operation yet. Add MHB with init
                                else {
                                    relations.push_back(make_pair("mhb", make_pair(lastGlobalInst, loadInst)));
                                }
                                if (lastGlobalOfVar[fromVarName])
                                    relations.push_back(make_pair("mhb", make_pair(lastGlobalOfVar[fromVarName], loadInst)));
                                lastGlobalOfVar[fromVarName] = loadInst;
                                lastGlobalInst = loadInst;
                            }
                        }
                    }
                }

            }
            // Save loads stores function wise
            allStores.emplace(func, varToStores);
            allLoads.emplace(func, varToLoads);
            
            // errs() << "Loads of function " << func->getName() << "\n";
            // for (auto it=varToLoads.begin(); it!=varToLoads.end(); ++it)
            //     printValue(it->first);

            Environment curFuncEnv;
            curFuncEnv.init(globalVars, funcVars);
            funcInitEnv[func] = curFuncEnv;
        }

        // errs() << "\nAll Loads:\n";
        // for(auto it1=allLoads.begin(); it1!=allLoads.end(); ++it1) {
        //     errs() << "Function " << it1->first->getName() << "\n";
        //     auto loadsOfFun = it1->second;
        //     for (auto it2=loadsOfFun.begin(); it2!=loadsOfFun.end(); ++it2) {
        //         printValue(it2->first);
        //     }
        // }
    
        getFeasibleInterferences(allLoads, allStores, relations);
        // printFeasibleInterf();
    }

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unsigned int iterations = 0;
        unordered_map <Function*, unordered_map<Instruction*, Environment>> programStateCurItr;
        bool isFixedPointReached = false;

        while (!isFixedPointReached) {
            programState = programStateCurItr;

            if (!noPrint) {
                errs() << "_________________________________________________\n";
                errs() << "Iteration: " << iterations << "\n";
                // printProgramState();
            }
            for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
                Function *curFunc = (*funcItr);
                if (!noPrint) {
                    errs() << "\n******** DEBUG: Analyzing thread " << curFunc->getName() << "********\n";
                }

                // find feasible interfernce for current function
                vector <unordered_map <Instruction*, Instruction*>> curFuncInterfs;
                unordered_map<Instruction*, Environment> newFuncEnv;

                auto searchInterf = feasibleInterfences.find(curFunc);
                
                if (searchInterf != feasibleInterfences.end()) {
                    curFuncInterfs = (searchInterf->second);
                } else {
                    if (!noPrint) errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        unordered_map <Instruction*, Instruction*> interf;
                        newFuncEnv = analyzeThread(curFunc, interf);
                        programStateCurItr.emplace(curFunc, newFuncEnv);
                    }
                }
                
                // errs() << "Number of interf= " << curFuncInterfs.size();
                // analyze the Thread for each interference
                for (auto interfItr=curFuncInterfs.begin(); interfItr!=curFuncInterfs.end(); ++interfItr){
                    // errs() << "\n***Forinterf\n";

                    newFuncEnv = analyzeThread(*funcItr, *interfItr);

                    // join newFuncEnv of all feasibleInterfs and replace old one in state
                    auto searchFunEnv = programStateCurItr.find(curFunc);
                    if (searchFunEnv == programStateCurItr.end()) {
                        // errs() << "curfunc not found in program state\n";
                        programStateCurItr.emplace(curFunc, newFuncEnv);
                    }
                    else {
                        // errs() << "curfunc already exist in program state. joining\n";
                        programStateCurItr[curFunc] =  joinEnvByInstruction(searchFunEnv->second, newFuncEnv);
                    }
                }
            }
            isFixedPointReached = true;
            isFixedPointReached = isFixedPoint(programStateCurItr);
            iterations++;
        }
        if (!noPrint) {
            errs() << "_________________________________________________\n";
            errs() << "Fized point reached in " << iterations << " iteratons\n";
            errs() << "Final domain:\n";
            printProgramState();
        }
    }

    unordered_map<Instruction*, Environment> analyzeThread (Function *F, unordered_map<Instruction*, Instruction*> interf) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, Environment> curFuncEnv;
        curFuncEnv[&(*(F->begin()->begin()))] = funcInitEnv[F];
        errs() << "CurDuncEnv before checking preds:\n";
        printInstToEnvMap(curFuncEnv);

        for(auto bbItr=F->begin(); bbItr!=F->end(); ++bbItr){
            BasicBlock *currentBB = &(*bbItr);

            Environment predEnv = funcInitEnv[F];
            
            // initial domain of pred of cur bb to join of all it's pred
            for (BasicBlock *Pred : predecessors(currentBB)){
                auto searchPredBBEnv = curFuncEnv.find(Pred->getTerminator());
                if (searchPredBBEnv != curFuncEnv.end())
                    predEnv.joinEnvironment(searchPredBBEnv->second);
                // TODO: if the domain is empty? It means pred bb has not been analyzed so far
                // we can't analyze current BB
            }
            // if termination statement
                // if coditional branching
                // if unconditional branching

            errs() << "CurDuncEnv before calling analyzeBB:\n";
            printInstToEnvMap(curFuncEnv);
            analyzeBasicBlock(currentBB, curFuncEnv, interf);
        }

        return curFuncEnv;
    }

    Environment analyzeBasicBlock (BasicBlock *B, 
        unordered_map <Instruction*, Environment> &curFuncEnv,
        unordered_map<Instruction*, Instruction*> interf
    ) {
        // check type of inst, and performTrasformations
        Environment curEnv;
        Environment predEnv = curFuncEnv[&(*(B->begin()))];
        // cmp instr will add the corresponding true and false branch environment to branchEnv. 
        // cmpVar -> (true branch env, false branch env)
        map<Instruction*, pair<Environment, Environment>> branchEnv;
        
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);
            curEnv.copyEnvironment(predEnv);
        
            if (!noPrint) {
                errs() << "DEBUG: Analyzing: ";
                printValue(currentInst);
            }

            if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(currentInst)) {
                // auto searchName = valueToName.find(currentInst);
                // if (searchName == valueToName.end()) {
                //     fprintf(stderr, "ERROR: new mem alloca");
                //     allocaInst->dump();
                //     exit(0);
                // }
            }
            else if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(instItr)) {
                auto oper = binOp->getOpcode();
                if (oper == Instruction::And || oper == Instruction::Or) {
                    curEnv = checkLogicalInstruction(binOp, curEnv, branchEnv);
                    // errs() << "True branch Env:\n";
                    // branchEnv[&(*instItr)].first.printEnvironment();
                    // errs() << "False branch Env:\n";
                    // branchEnv[&(*instItr)].second.printEnvironment();
                }
                else curEnv = checkBinInstruction(binOp, curEnv);
            }
            else if (StoreInst *storeInst = dyn_cast<StoreInst>(instItr)) {
                curEnv = checkStoreInst(storeInst, curEnv);
            }
            else if (UnaryInstruction *unaryInst = dyn_cast<UnaryInstruction>(instItr)) {
                curEnv = checkUnaryInst(unaryInst, curEnv, interf);
            }
            else if (CmpInst *cmpInst = dyn_cast<CmpInst> (instItr)) {
                // errs() << "cmpInst: ";
                // cmpInst->print(errs());
                // errs() << "\n";
                curEnv = checkCmpInst(cmpInst, curEnv, branchEnv);
                // errs() << "\nCmp result:\n";
                // curEnv.printEnvironment();
                // errs() <<"True Branch:\n";
                // branchEnv[cmpInst].first.printEnvironment();
                // errs() <<"Flase Branch:\n";
                // branchEnv[cmpInst].second.printEnvironment();
            }
            else if (BranchInst *branchInst = dyn_cast<BranchInst>(instItr)) {
                if (branchInst->isConditional()) {
                    Instruction *branchCondition = dyn_cast<Instruction>(branchInst->getCondition());
                    Instruction *trueBranch = &(*(branchInst->getSuccessor(0)->begin()));
                    curFuncEnv[trueBranch].joinEnvironment(branchEnv[branchCondition].first);
                    Instruction *falseBranch = &(*(branchInst->getSuccessor(1)->begin()));
                    curFuncEnv[falseBranch].joinEnvironment(branchEnv[branchCondition].second);
                    // errs() << "\nTrue Branch:\n";
                    // printValue(trueBranch);
                    // errs() << "True branch Env:\n";
                    // curFuncEnv[trueBranch].printEnvironment();
                    // errs() << "\nFalse Branch:\n";
                    // printValue(falseBranch);
                    // errs() << "False branch Env:\n";
                    // curFuncEnv[falseBranch].printEnvironment();
                }
                else {
                    Instruction *successors = &(*(branchInst->getSuccessor(0)->begin()));
                    curFuncEnv[successors].joinEnvironment(curEnv);
                }
            }
            else if (CallInst *callInst = dyn_cast<CallInst>(instItr)) {
                if (callInst->getCalledFunction()->getName() == "__assert_fail") {
                    // errs() << "*** found assert" << "\n";
                    // printValue(callInst);
                    if (!curEnv.isUnreachable()) {
                        errs() << "__________________________________________________\n";
                        errs() << "ERROR: Assertion failed\n";
                        if (!noPrint) {
                            printValue(callInst);
                            curEnv.printEnvironment();
                        }
                        exit(0);
                    }
                }
            }
            else {
                
            }
            // RMW, CMPXCHG

            curFuncEnv[currentInst] = curEnv;
            predEnv.copyEnvironment(curEnv);
            curEnv.printEnvironment();
        }
           
        return curEnv;
    }

    string getNameFromValue (Value *val) {
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

    Environment checkCmpInst (
        CmpInst* cmpInst, 
        Environment curEnv, 
        map<Instruction*, pair<Environment, Environment>> &branchEnv
    ) { 
        // need to computer Environment of both true and false branch
        Environment trueBranchEnv;
        Environment falseBranchEnv;
        trueBranchEnv.copyEnvironment(curEnv);
        falseBranchEnv.copyEnvironment(curEnv);

        operation operTrueBranch;
        operation operFalseBranch;

        switch (cmpInst->getPredicate()) {
            case CmpInst::Predicate::ICMP_EQ:
                operTrueBranch = EQ;
                operFalseBranch = NE;
                break;
            case CmpInst::Predicate::ICMP_NE:
                operTrueBranch = NE;
                operFalseBranch = EQ;
                break;
            case CmpInst::Predicate::ICMP_SGT:
                operTrueBranch = GT;
                operFalseBranch = LE;
                break;
            case CmpInst::Predicate::ICMP_SGE:
                operTrueBranch = GE;
                operFalseBranch = LT;
                break;
            case CmpInst::Predicate::ICMP_SLT:
                operTrueBranch = LT;
                operFalseBranch = GE;
                break;
            case CmpInst::Predicate::ICMP_SLE:
                operTrueBranch = LE;
                operFalseBranch = GT;
                break;
            default:
                if (!noPrint) {
                    errs() << "WARNING: Unknown cmp instruction: ";
                    printValue(cmpInst);
                }
                return curEnv;
        }

        string destVarName = getNameFromValue(cmpInst);
        Value* fromVar1 = cmpInst->getOperand(0);
        Value* fromVar2 = cmpInst->getOperand(1);
        // errs() << "destVarName: " << destVarName << "\n";
        // errs() << "fromVar1: ";
        // fromVar1->print(errs());
        // errs() << "\n";
        // errs() << "fromVar2: ";
        // fromVar2->print(errs());
        // errs() << "\n";

        if (ConstantInt *constFromVar1 = dyn_cast<ConstantInt>(fromVar1)) {
            int constFromIntVar1= constFromVar1->getValue().getSExtValue();
            if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
                int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
                trueBranchEnv.performCmpOp(operTrueBranch, constFromIntVar1, constFromIntVar2);
                falseBranchEnv.performCmpOp(operFalseBranch, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                trueBranchEnv.performCmpOp(operTrueBranch, constFromIntVar1, fromVar2Name);
                falseBranchEnv.performCmpOp(operFalseBranch, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            trueBranchEnv.performCmpOp(operTrueBranch, fromVar1Name, constFromIntVar2);
            falseBranchEnv.performCmpOp(operFalseBranch, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            trueBranchEnv.performCmpOp(operTrueBranch, fromVar1Name, fromVar2Name);
            falseBranchEnv.performCmpOp(operFalseBranch, fromVar1Name, fromVar2Name);
        }

        // set the value of destination variable in Environment
        // if (trueBranchEnv.isUnreachable()) {
        //     curEnv.unsetVar(destVarName);
        // } else {
        //     curEnv.setVar(destVarName);
        // }
        // trueBranchEnv.setVar(destVarName);
        // falseBranchEnv.unsetVar(destVarName);

        branchEnv[cmpInst] = make_pair(trueBranchEnv, falseBranchEnv);
        return curEnv;
    }

    Environment checkLogicalInstruction (
        BinaryOperator* logicalOp, 
        Environment curEnv, 
        map<Instruction*, pair<Environment, Environment>> &branchEnv
    ) {
        string destVarName = getNameFromValue(logicalOp);
        Value* fromVar1 = logicalOp->getOperand(0);
        Value* fromVar2 = logicalOp->getOperand(1);
        
        Environment fromVar1TrueEnv;
        Environment fromVar1FalseEnv;
        Environment fromVar2TrueEnv;
        Environment fromVar2FalseEnv;
        
        if (CmpInst *op1 = dyn_cast<CmpInst>(fromVar1)) {
            auto env = branchEnv[op1];
            fromVar1TrueEnv = env.first;
            fromVar1FalseEnv = env.second;
        }
        else if (BinaryOperator *op1 = dyn_cast<BinaryOperator>(fromVar1)) {
            auto oper = op1->getOpcode();
            if (oper == Instruction::And || oper == Instruction::Or) {
                auto env = branchEnv[op1];
                fromVar1TrueEnv = env.first;
                fromVar1FalseEnv = env.second;
            }
        }
        else {
            errs() << "ERROR: env not found in branchEnv\n";
            exit(0);
        }
        if (CmpInst *op2 = dyn_cast<CmpInst>(fromVar2)) {
            auto env = branchEnv[op2];
            fromVar2TrueEnv = env.first;
            fromVar2FalseEnv = env.second;
            // auto searchOp2Branches = branchEnv.find(op2);
            // if (searchOp2Branches == branchEnv.end()) {
            //     errs() << "!!!! Something went wrong !!!!\n";
            //     exit(0);
            // }
            // fromVar2TrueEnv = searchOp2Branches->second.first;
            // errs() << "T2:\n";
            // fromVar2TrueEnv.printEnvironment();
            // fromVar2FalseEnv = searchOp2Branches->second.second;
            // errs() << "F2:\n";
            // fromVar2FalseEnv.printEnvironment();
        }
        else if (BinaryOperator *op2 = dyn_cast<BinaryOperator>(fromVar2)) {
            auto oper = op2->getOpcode();
            if (oper == Instruction::And || oper == Instruction::Or) {
                auto env = branchEnv[op2];
                fromVar2TrueEnv = env.first;
                fromVar2FalseEnv = env.second;
            }
        }
        else {
            errs() << "ERROR: env not found in branchEnv\n";
            exit(0);
        }

        // since we are working with -O1, none of the operands can be constant.
        // string fromVar1Name = getNameFromValue(fromVar1);
        // string fromVar2Name = getNameFromValue(fromVar2);

        auto oper = logicalOp->getOpcode();
        Environment trueBranchEnv;
        Environment falseBranchEnv;
        trueBranchEnv.copyEnvironment(fromVar1TrueEnv);
        falseBranchEnv.copyEnvironment(fromVar1FalseEnv);
        if (oper == Instruction::And) {
            trueBranchEnv.meetEnvironment(zHelper, fromVar2TrueEnv);
            falseBranchEnv.joinEnvironment(fromVar2FalseEnv);
        }

        else if (oper == Instruction::Or) {
            //-
            // errs() << "T1:\n";
            // trueBranchEnv.printEnvironment();
            // errs() << "T2:\n";
            // fromVar2TrueEnv.printEnvironment();
            //-//
            trueBranchEnv.joinEnvironment(fromVar2TrueEnv);
            // errs() << "T1 join T2:\n";
            // trueBranchEnv.printEnvironment();

            //-
            // fromVar2TrueEnv.joinEnvironment(fromVar1TrueEnv);
            // errs() << "T2 join T1:\n";
            // fromVar2TrueEnv.printEnvironment();

            // errs() << "F1:\n";
            // falseBranchEnv.printEnvironment();
            // errs() << "F2:\n";
            // fromVar2FalseEnv.printEnvironment();

            // fromVar2FalseEnv.meetEnvironment(fromVar1FalseEnv);
            // errs() << "F2 meet F1:\n";
            // fromVar2FalseEnv.printEnvironment();
            //-//
            falseBranchEnv.meetEnvironment(zHelper, fromVar2FalseEnv);
            // errs() << "F1 meet F2:\n";
            // falseBranchEnv.printEnvironment();
        }

        else {
            if (!noPrint) {
                errs() << "WARNING: unknown binary operation: ";
                printValue(logicalOp);
            }
            return curEnv;
        }

        // set the value of destination variable in Environment
        // if (trueBranchEnv.isUnreachable()) {
        //     curEnv.unsetVar(destVarName);
        // } else {
        //     curEnv.setVar(destVarName);
        // }
        // trueBranchEnv.setVar(destVarName);
        // falseBranchEnv.unsetVar(destVarName);

        branchEnv[logicalOp] = make_pair(trueBranchEnv, falseBranchEnv);
        return curEnv;
    }

    //  call approprproate function for the inst passed
    Environment checkBinInstruction(BinaryOperator* binOp, Environment curEnv) {
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
                if (!noPrint) {
                    errs() << "WARNING: unknown binary operation: ";
                    printValue(binOp);
                }
                return curEnv;
        }

        string destVarName = getNameFromValue(binOp);
        Value* fromVar1 = binOp->getOperand(0);
        Value* fromVar2 = binOp->getOperand(1);
        if (ConstantInt *constFromVar1 = dyn_cast<ConstantInt>(fromVar1)) {
            int constFromIntVar1= constFromVar1->getValue().getSExtValue();
            if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
                int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
                curEnv.performBinaryOp(oper, destVarName, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                curEnv.performBinaryOp(oper, destVarName, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            curEnv.performBinaryOp(oper, destVarName, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            curEnv.performBinaryOp(oper, destVarName, fromVar1Name, fromVar2Name);
        }

        return curEnv;
    }

    Environment checkStoreInst(StoreInst* storeInst, Environment curEnv) {
        Value* destVar = storeInst->getPointerOperand();
        string destVarName = getNameFromValue(destVar);

        auto ord = storeInst->getOrdering();
        if (ord==llvm::AtomicOrdering::Release || 
            ord==llvm::AtomicOrdering::SequentiallyConsistent ||
            ord==llvm::AtomicOrdering::AcquireRelease) {
            // if (curEnv.getRelHead(destVarName) == nullptr)
            //     curEnv.setRelHead(destVarName, storeInst);
            // curEnv.changeRelHeadIfNull(destVarName, storeInst);
            if(useMOPO) {
                errs() << "appending " << storeInst << " to " << destVarName << "\n";
                curEnv.appendInst(zHelper, storeInst, destVarName);
            }
        }
        else {
            // curEnv.changeRelHeadToNull(destVarName, storeInst);
            // since we are assuming only RA, this block is not required.
        }

        Value* fromVar = storeInst->getValueOperand();
        
        if (ConstantInt *constFromVar = dyn_cast<ConstantInt>(fromVar)) {
            int constFromIntVar = constFromVar->getValue().getSExtValue();
            curEnv.performUnaryOp(STORE, destVarName, constFromIntVar);
        }
        else if (Argument *argFromVar = dyn_cast<Argument>(fromVar)) {
            // TODO: handle function arguments

        }
        else {
            string fromVarName = getNameFromValue(fromVar);
            curEnv.performUnaryOp(STORE, destVarName, fromVarName);
        }
        // curEnv.printEnvironment();
        return curEnv;
    }

    Environment checkUnaryInst(
        UnaryInstruction* unaryInst, 
        Environment curEnv, 
        unordered_map<Instruction*, Instruction*> interf
    ) {
        Value* fromVar = unaryInst->getOperand(0);
        string fromVarName = getNameFromValue(fromVar);
        string destVarName = getNameFromValue(unaryInst);
        
        operation oper;
        switch (unaryInst->getOpcode()) {
            case Instruction::Load:
                oper = LOAD;
                // Apply interfernce before analyzing the instruction
                if(GEPOperator *gepOp = dyn_cast<GEPOperator>(fromVar)){
                    fromVar = gepOp->getPointerOperand();
                }           
                if (dyn_cast<GlobalVariable>(fromVar)) {
                    // errs() << "Load of global\n";
                    // errs() << "Env before:";
                    // curEnv.printEnvironment();
                    curEnv = applyInterfToLoad(unaryInst, curEnv, interf, fromVarName);
                }
                break;
            // TODO: add more cases
            default: 
                if (!noPrint) {
                    fprintf(stderr, "ERROR: unknown operation: ");
                    unaryInst->print(errs());
                }
                return curEnv;
        }
        
        curEnv.performUnaryOp(oper, destVarName.c_str(), fromVarName.c_str());

        return curEnv;
    }

    Environment applyInterfToLoad(
        UnaryInstruction* unaryInst, 
        Environment curEnv, 
        unordered_map<Instruction*, Instruction*> interf,
        string varName
    ) {
        // errs() << "Applying interf\n";
        // find interfering instruction
        auto searchInterf = interf.find(unaryInst);
        if (searchInterf == interf.end()) {
            errs() << "ERROR: Interfernce for the load instrction not found\n";
            printValue(unaryInst);
            return curEnv;
        }
        Instruction *interfInst = searchInterf->second;
        
        // if interfernce is from some other thread
        if (interfInst && interfInst != unaryInst->getPrevNode()) {
            // find the domain of interfering instruction
            auto searchInterfFunc = programState.find(interfInst->getFunction());
            if (searchInterfFunc != programState.end()) {
                auto searchInterfEnv = searchInterfFunc->second.find(interfInst);
                // errs() << "For Load: ";
                // unaryInst->print(errs());
                if (!noPrint) {
                    errs() << "\nInterf with Store: ";
                    interfInst->print(errs());
                    errs() << "\n";
                }
                if (searchInterfEnv != searchInterfFunc->second.end()) {
                    // apply the interference
                    // errs() << "Before Interf:\n";
                    // curEnv.printEnvironment();

                    Environment interfEnv = searchInterfEnv->second;
                    bool isRelSeq = false;
                    if (StoreInst *storeInst = dyn_cast<StoreInst>(interfInst)) {
                        if (LoadInst *loadInst = dyn_cast<LoadInst>(unaryInst)) {
                            auto ordStore = storeInst->getOrdering();
                            auto ordLoad  = loadInst->getOrdering();
                            if (ordLoad==llvm::AtomicOrdering::Acquire || 
                                ordLoad==llvm::AtomicOrdering::SequentiallyConsistent ||
                                ordLoad==llvm::AtomicOrdering::AcquireRelease) {
                                if (ordStore==llvm::AtomicOrdering::Release ||
                                    ordStore==llvm::AtomicOrdering::SequentiallyConsistent ||
                                    ordStore==llvm::AtomicOrdering::AcquireRelease) {
                                    isRelSeq = true;
                                }
                                else if (prevRelWriteOfSameVar[storeInst]){
                                    curEnv.carryEnvironment(varName, interfEnv);
                                }
                            }
                        }
                    }
                    curEnv.applyInterference(varName, interfEnv, isRelSeq, zHelper, interfInst);
                }
            }
        }
        return curEnv;
    }

    unordered_map<Function*, vector< unordered_map<Instruction*, Instruction*>>> getAllInterferences (
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores
    ){
        unordered_map<Function*, vector< unordered_map<Instruction*, Instruction*>>> allInterfs;

        for (auto funItr=loadsToAllStores.begin(); funItr!=loadsToAllStores.end(); ++funItr) {
            Function *curFunc = funItr->first;
            auto allLS = funItr->second;
            Instruction* loads[allLS.size()];
            vector<Instruction*>::iterator allItr[allLS.size()];
            int noOfInterfs = 1;
            int i=0;
            for (auto itr=allLS.begin(); itr!=allLS.end(); ++itr, i++) {
                loads[i] = itr->first;
                allItr[i] = itr->second.begin();
                if (!itr->second.empty()) noOfInterfs *= itr->second.size();
            }
            
            unordered_map<Instruction*, Instruction*> curInterf;
            
            for (int i=0; i<noOfInterfs; i++) {
                for (int j=0; j<allLS.size(); j++) {
                    if (allItr[j] != allLS[loads[j]].end()) {
                        curInterf[loads[j]] = (*allItr[j]);
                    }
                }
                allInterfs[curFunc].push_back(curInterf);
                int k = allLS.size()-1;
                if (allItr[k] != allLS[loads[k]].end()) {
                    allItr[k]++;
                }
                while (allItr[k] == allLS[loads[k]].end()) {
                    allItr[k] = allLS[loads[k]].begin();
                    k--;
                    if (k>=0) allItr[k]++;
                }
            }
        }
        
        return allInterfs;
    }

    void getFeasibleInterferences (
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores, 
        vector< pair <string, pair<Instruction*, Instruction*>>> relations
    ){
        unordered_map<Function*, vector< unordered_map<Instruction*, Instruction*>>> allInterfs;
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores;
        // Make all permutations
        loadsToAllStores = getLoadsToAllStoresMap(allLoads, allStores);
        allInterfs = getAllInterferences(loadsToAllStores);

        // Check feasibility of permutations and save them in feasibleInterfences
        if (useZ3) {
            #pragma omp parallel num_threads(omp_get_num_procs()*2)
            #pragma omp single 
            {
            for (auto funcItr=allInterfs.begin(); funcItr!=allInterfs.end(); ++funcItr) {
                vector< unordered_map<Instruction*, Instruction*>> curFuncInterfs;
                for (auto interfItr=funcItr->second.begin(); interfItr!=funcItr->second.end(); ++interfItr) {
                    auto interfs = *interfItr;
                    #pragma omp task private(interfs) shared(curFuncInterfs)
                    {
                        // int tid = omp_get_thread_num();
                        // errs() << "from " << tid << "\n";
                        bool feasible = true;
                        feasible = isFeasible(*interfItr, allLoads, allStores, relations);
                        if (feasible) {
                            curFuncInterfs.push_back(*interfItr);
                        }
                    }
                }
                #pragma omp taskwait
                feasibleInterfences[funcItr->first] = curFuncInterfs;
            }
            }
        }
        else if (minimalZ3) {
            for (auto funcItr=allInterfs.begin(); funcItr!=allInterfs.end(); ++funcItr) {
                vector< unordered_map<Instruction*, Instruction*>> curFuncInterfs;
                for (auto interfItr=funcItr->second.begin(); interfItr!=funcItr->second.end(); ++interfItr) {
                    auto interfs = *interfItr;
                    if (isFeasibleMinimal(*interfItr))
                        curFuncInterfs.push_back(*interfItr);
                }
                feasibleInterfences[funcItr->first] = curFuncInterfs;
            }
        }
        else {
            feasibleInterfences = allInterfs;
        }
    }

    bool isFeasible (
        unordered_map<Instruction*, Instruction*> interfs, 
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores,
        vector< pair <string, pair<Instruction*, Instruction*>>> relations
    ) {
        // errs() << "isFeasible - checking interf:\n";
        // for (auto it=interfs.begin(); it!=interfs.end(); ++it) {
        //     it->first->print(errs());
        //     errs() << "\n\t--Reads from-->\t";
        //     if (it->second == nullptr) errs() << "init";
        //     else it->second->print(errs());
        //     errs() << "\n";
        // }

        Z3Helper checker;
        checker.addInferenceRules();
        checker.addMHBandPORules(relations);
        checker.addAllLoads(allLoads);
        checker.addAllStores(allStores);
        return checker.checkInterference(interfs);
        
        // checker.testQuery();
        // return true;
    }

    bool isFeasibleMinimal(unordered_map<Instruction*, Instruction*> interfs) {
        for (auto lsPair=interfs.begin(); lsPair!=interfs.end(); ++lsPair) {
            if (lsPair->second == nullptr)
                continue;
            for (auto otherLS=interfs.begin(); otherLS!=interfs.end(); ++otherLS) {
                // lsPair: (s --rf--> l), otherLS: (s' --rf--> l')
                if (otherLS == lsPair || otherLS->second==nullptr)
                    continue;
                LoadInst  *ld = dyn_cast<LoadInst> (lsPair->first);
                StoreInst *st = dyn_cast<StoreInst>(lsPair->second);
                LoadInst  *ld_prime = dyn_cast<LoadInst> (otherLS->first);
                StoreInst *st_prime = dyn_cast<StoreInst>(otherLS->second);        
                // (l --sb--> l')
                if (zHelper.querySB(ld, ld_prime)) {
                    // (l --sb--> l' && s = s') reading from local context will give the same result
                    if (st == st_prime) return false;
                    
                    auto ordStore = st->getOrdering();
                    auto ordLoad  = ld->getOrdering();
                    Value* obj = st->getPointerOperand();
                    if(GEPOperator *gepOp = dyn_cast<GEPOperator>(obj)){
                        obj = gepOp->getPointerOperand();
                    }
                    Value* obj_prime = st_prime->getPointerOperand();
                    if(GEPOperator *gepOp = dyn_cast<GEPOperator>(obj_prime)){
                        obj_prime = gepOp->getPointerOperand();
                    }

                    if ((ordLoad == llvm::AtomicOrdering::Acquire ||
                            ordLoad == llvm::AtomicOrdering::AcquireRelease ||
                            ordLoad == llvm::AtomicOrdering::SequentiallyConsistent) && 
                            (ordStore == llvm::AtomicOrdering::Release ||
                            ordStore == llvm::AtomicOrdering::AcquireRelease ||
                            ordStore == llvm::AtomicOrdering::SequentiallyConsistent)) {
                        if (zHelper.querySB(st_prime, st)) return false;
                    }
                    else if (ordLoad == llvm::AtomicOrdering::Acquire ||
                            ordLoad == llvm::AtomicOrdering::AcquireRelease ||
                            ordLoad == llvm::AtomicOrdering::SequentiallyConsistent) {
                        StoreInst *st_pre = prevRelWriteOfSameVar[st];
                        if (st_pre && st_pre==st_prime) return false;
                        else if (obj == obj_prime && zHelper.querySB(st_prime, st)) return false;
                        else if (st_pre && zHelper.querySB(st_prime, st_pre)) return false;
                    }
                    else if (obj == obj_prime && zHelper.querySB(st_prime, st)) return false;
                }
            }
        }
        return true;
    }

    unordered_map<Instruction*, Environment> joinEnvByInstruction (
        unordered_map<Instruction*, Environment> instrToEnvOld,
        unordered_map<Instruction*, Environment> instrToEnvNew
    ) {
        // new = old join new
        for (auto itOld=instrToEnvOld.begin(); itOld!=instrToEnvOld.end(); ++itOld) {
            // errs() << "joining for instruction: ";
            // itOld->first->print(errs());
            // errs() << "\n";
            auto searchNewMap = instrToEnvNew.find(itOld->first);
            if (searchNewMap == instrToEnvNew.end()) {
                instrToEnvNew[itOld->first] = itOld->second;
            } else {
                Environment newEnv = searchNewMap->second;
                newEnv.joinEnvironment(itOld->second);
                instrToEnvNew[itOld->first] = newEnv;
            }
        }
        
        return instrToEnvNew;
    }

    void checkAssertions() {
        int num_errors = 0;
        for (auto it1=programState.begin(); it1!=programState.end(); ++it1) {
            for (auto it=it1->second.begin(); it!=it1->second.end(); ++it) {
                if (CallInst *callInst = dyn_cast<CallInst>(it->first)) {
                    if (callInst->getCalledFunction()->getName() == "__assert_fail") {
                        // errs() << "*** found assert" << "\n";
                        // printValue(callInst);
                        Environment curEnv = it->second;
                        if (!curEnv.isUnreachable()) {
                            errs() << "ERROR: Assertion failed\n";
                            printValue(callInst);
                            if (!noPrint)
                                curEnv.printEnvironment();
                            num_errors++;
                        }
                    }
                }
            }
        }
        errs() << "___________________________________________________\n";
        errs() << "Errors Found: " << num_errors << "\n";
    }

    void printLoadsToAllStores(unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores){
        errs() << "All load-store pair in the program\n";
        for (auto it1=loadsToAllStores.begin(); it1!=loadsToAllStores.end(); ++it1) {
            errs () << "***Function " << it1->first->getName() << ":\n";
            auto l2s = it1->second;
            for (auto it2=l2s.begin(); it2!=l2s.end(); ++it2) {
                errs() << "Stores for Load: ";
                printValue(it2->first);
                auto stores = it2->second;
                for (auto it3=stores.begin(); it3!=stores.end(); ++it3) {
                    errs() << "\t";
                    if(*it3) printValue(*it3);
                    else errs() << "Thread Local Context\n";
                }
            }
        }
    }

    void testApplyInterf() {
        // Sample code to test applyInterference()
        auto funIt = funcInitEnv.begin();
        Function *fun1 = funIt->first;
        Environment fun1Env = funIt->second;
        fun1Env.performUnaryOp(STORE, "x", 1);
        fun1Env.performUnaryOp(STORE, "y", 10);
        funIt++;
        Function *fun2 = funIt->first;
        Environment fun2Env = funIt->second;
        errs() << "Interf from domain:\n";
        fun1Env.printEnvironment();
        errs() << "To domain:\n";
        fun2Env.printEnvironment();
        fun2Env.applyInterference("x", fun1Env, true, zHelper);
        errs() << "After applying:\n";
        fun2Env.printEnvironment();
    }

    void testPO() {
        /* Test using test32 */
        PartialOrder po, po2;
        auto p1=prevRelWriteOfSameVar.begin()->first;
        auto p2=prevRelWriteOfSameVar.begin()->first;
        auto p3=prevRelWriteOfSameVar.begin()->first;
        bool first = true, append=true;
        for (auto it=prevRelWriteOfSameVar.begin(); it!=prevRelWriteOfSameVar.end(); it++) {
            printValue(it->first);
            printValue(it->second);
            errs() << "\n";
            if (it->first!=nullptr && it->second!=nullptr) {
                errs() << "Adding PO between "; printValue(it->first); printValue(it->second);
                po.addOrder(zHelper, it->first, it->second);
                first ? (p1 = it->second) : (first=false, p1= it->first);
                // break;
            } 
            else if (!append) {p3 = ((it->first!=nullptr)?(it->first):(it->second)); append=false;}
        }
        errs() << po.toString() << "\n"; 

        errs() << "second po\n";
        errs() << p1 << " order with " << p2 << ": " << po2.addOrder(zHelper, p1,p2) << "\n";
        errs() << po2.toString() << "\n";    

        errs() << p1 << " isOrderedBefore " << p2 << ": " << po.isOrderedBefore(p1,p2) << "\n";
        errs() << p2 << " isOrderedBefore " << p1 << ": " << po.isOrderedBefore(p2,p1) << "\n";
        errs() << p1 << " isOrderedBefore " << p2 << ": " << po2.isOrderedBefore(p1,p2) << "\n";
        errs() << p2 << " isOrderedBefore " << p1 << ": " << po2.isOrderedBefore(p2,p1) << "\n";

        errs() << "\npo Join po2\n";
        errs() << po.join(zHelper, po2) << "\n";
        errs() << po.toString() << "\n";

        errs() << p1 << " isOrderedBefore " << p2 << ": " << po.isOrderedBefore(p1,p2) << "\n";
        errs() << p2 << " isOrderedBefore " << p1 << ": " << po.isOrderedBefore(p2,p1) << "\n";
        errs() << p1 << " isOrderedBefore " << p2 << ": " << po2.isOrderedBefore(p1,p2) << "\n";
        errs() << p2 << " isOrderedBefore " << p1 << ": " << po2.isOrderedBefore(p2,p1) << "\n";

        errs() << "append " << p2 << ": " << po.append(zHelper, p2) << "\n";
        errs() << po.toString() << "\n";

        errs() << "removing " << p1 << "\n";
        po.remove(p1);
        errs() << po.toString() << "\n";

        errs() << "Append " << p1 << "again\n";
        po.append(zHelper, p1);
        errs() << po.toString() << "\n";
        
    }

    void printInstToEnvMap(unordered_map<Instruction*, Environment> instToEnvMap) {
        for (auto it=instToEnvMap.begin(); it!=instToEnvMap.end(); ++it) {
            it->first->print(errs());
            it->second.printEnvironment();
        }
    }

    void printProgramState() {
        for (auto it1=programState.begin(); it1!=programState.end(); ++it1) {
            errs() << "\n-----------------------------------------------\n";
            errs() << "Function " << it1->first->getName() << ":\n";
            errs() << "-----------------------------------------------\n";
            printInstToEnvMap(it1->second);
        }
    }

    bool isFixedPoint(unordered_map <Function*, unordered_map<Instruction*, Environment>> newProgramState) {
        // errs() << "Program state size: " << programState.size();
        // errs() << "\nnew program state size: " << newProgramState.size()<<"\n";
        return (newProgramState == programState);
    }

    void printFeasibleInterf() {
        errs() << "\nFeasible Interfs\n";
        for (auto it1=feasibleInterfences.begin(); it1!=feasibleInterfences.end(); ++it1) {
            errs() << "Interfs for function: " << it1->first->getName();
            auto allInterfOfFun = it1->second;
            int i = 0;
            for (auto it2=allInterfOfFun.begin(); it2!=allInterfOfFun.end(); ++it2, ++i) {
                errs() << "Interf " << i << ":\n";
                for (auto it3=it2->begin(); it3!=it2->end(); ++it3) {
                    errs() << "\tLoad: ";
                    it3->first->print(errs());
                    errs() << "\n\tStore: ";
                    if (it3->second) it3->second->print(errs());
                    else errs() << "Thread Local Context";
                    errs() << "\n";
                }
            }
        }
    }

    bool isGlobalLoad(Instruction *inst) {
        if (LoadInst *loadInst = dyn_cast<LoadInst> (inst)) {
            Value* fromVar = loadInst->getOperand(0);
            if(GEPOperator *gepOp = dyn_cast<GEPOperator>(fromVar)){
                fromVar = gepOp->getPointerOperand();
            }
            if (dyn_cast<GlobalVariable>(fromVar)) {
                return true;
            }
        }
        return false;
    }

    bool isGlobalStore(Instruction *inst) {
        if(StoreInst *storeInst = dyn_cast<StoreInst>(inst)) {
            Value* destVar = storeInst->getPointerOperand();
            if(GEPOperator *gepOp = dyn_cast<GEPOperator>(destVar)){
                destVar = gepOp->getPointerOperand();
            }
            if (dyn_cast<GlobalVariable>(destVar)) {
                return true;
            }
        }
        return false;
    }

    bool isPthreadCreate(Instruction *inst) {
        if (CallInst *call = dyn_cast<CallInst>(inst)) {
            if(!call->getCalledFunction()->getName().compare("pthread_create")) {
                return true;
            }
        }
        return false;
    }

    bool isPthreadJoin(Instruction *inst) {
        if (CallInst *call = dyn_cast<CallInst>(inst)) {
            if(!call->getCalledFunction()->getName().compare("pthread_join")) {
                return true;
            }
        }
        return false;
    }

    Instruction* getLastGlobalInst(Instruction *inst) {
        Instruction *prevInst = inst->getPrevNode();
        while ( prevInst                    &&
                ! isGlobalLoad(prevInst)    &&
                ! isGlobalStore(prevInst)   &&
                ! isPthreadCreate(prevInst) &&
                ! isPthreadJoin(prevInst)) {
            prevInst = prevInst->getPrevNode();
        } 
        return prevInst;
    }

    Instruction* getNextGlobalInst(Instruction *inst) {
        Instruction *nextInst = inst;
        while ( nextInst                    &&
                ! isGlobalLoad(nextInst)    &&
                ! isGlobalStore(nextInst)   &&
                ! isPthreadCreate(nextInst) &&
                ! isPthreadJoin(nextInst)) {
            nextInst = nextInst->getNextNode ();
        }
        return nextInst;
    }

    Function* findFunctionFromPthreadJoin(Instruction* call) {
        Value* threadOp = call->getOperand(0);
        if (LoadInst *loadInst = dyn_cast<LoadInst>(threadOp))
            threadOp = loadInst->getPointerOperand();
        for (auto it=threadOp->user_begin(); it!=threadOp->user_end(); ++it) {
            if (CallInst *callInst = dyn_cast<CallInst>(*it)) {
                if (callInst->getCalledFunction()->getName() == "pthread_create") {
                    if (Function* func = dyn_cast<Function>(callInst->getArgOperand(2)))
                        return func;
                }
            }
        }
        return nullptr;
    }


    vector<Instruction*> getLastInstOfPthreadJoin(Instruction *call) {
        vector<Instruction*> lastInstr;
        Function *func = findFunctionFromPthreadJoin(call);
        for (auto bbItr=func->begin(); bbItr!=func->end(); ++bbItr) {
            TerminatorInst *term = bbItr->getTerminator();
            if (ReturnInst *ret = dyn_cast<ReturnInst>(term)) {
                lastInstr.push_back(getLastGlobalInst(ret));
            }
        }
        return lastInstr;
    }

    void printValue(Value *val) {
        if (val!=nullptr) {
            val->print(errs());
            errs() << "\n";
        }
        else errs() << "nullptr\n";
    }

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstract Interpretation Verifier Pass", false, true);
