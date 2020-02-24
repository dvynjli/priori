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
// cl::opt<bool> useZ3     ("z3", cl::desc("Enable interferce pruning using Z3"));
cl::opt<bool> noPrint   ("no-print", cl::desc("Do not print debug output"));
cl::opt<bool> minimalZ3 ("z3-minimal", cl::desc("Enable interference pruning using Z3"));
cl::opt<bool> useMOHead ("useMOHead", cl::desc("Enable interference pruning using Z3 using modification order head based analysis"));
cl::opt<bool> useMOPO ("useMOPO", cl::desc("Enable interference pruning using Z3 using partial order over modification order based analysis"));

map<llvm::Instruction*, pair<unsigned short int, unsigned int>> instToNum;
map<pair<unsigned short int, unsigned int>, llvm::Instruction*> numToInst;

class VerifierPass : public ModulePass {

    typedef EnvironmentPOMO Environment;
    // typedef EnvironmentRelHead Environment;

    vector <Function*> threads;
    unordered_map <Function*, unordered_map<Instruction*, Environment>> programState;
    // initial environment of the function. A map from Func-->(isChanged, Environment)
    unordered_map <Function*, pair<bool,Environment>> funcInitEnv;
    map <Function*, vector< map<Instruction*, Instruction*>>> feasibleInterfences;
    Z3Minimal zHelper;

    unordered_map <string, Value*> nameToValue;
    unordered_map <Value*, string> valueToName;
    map<Instruction*, map<string, Instruction*>> lastWrites; 
    
    vector<string> globalVars;
    unsigned int iterations = 0;

    #ifdef NOTRA
    map<StoreInst*, StoreInst*> prevRelWriteOfSameVar;
    #endif

    #ifdef ALIAS
    map<Function*, AliasAnalysis*> aliasAnalyses;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.addRequired<AAResultsWrapperPass>();
    }
    #endif


    bool runOnModule (Module &M) {
        double start_time = omp_get_wtime();
        globalVars = getGlobalIntVars(M);
        // errs() << "#global vars:" << globalVars.size() << "\n";
        // zHelper.initZ3(globalVars);
        initThreadDetails(M);
        // printFeasibleInterf();
        // printNumFeasibleInterf();
        // printInstMaps();
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
        fprintf(stderr, "#iterations: %d\n", iterations);
        return true;
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
                // errs() << "added the global var\n";

            }
            // Pointers are not needed to be kept as global variables
            /* else if (PointerType* ptrTy = dyn_cast<PointerType>(it->getValueType())) {
                // errs() << "Pointer: "; ptrTy->getElementType()->print(errs()); errs() << "\n";
                if (StructType* structTy = dyn_cast<StructType>(ptrTy->getElementType())) {
                    if  (!structTy->getName().compare("struct.std::atomic")) {
                        string varName = it->getName();
                        intVars.push_back(varName);
                        Value * varInst = &(*it);
                        nameToValue.emplace(varName, varInst);
                        valueToName.emplace(varInst, varName);
                    }
                }
            } */
            else if (StructType* structTy = dyn_cast<StructType>(it->getValueType())) {
                if  (!structTy->getName().find("struct.std::atomic")) {
                    string varName = it->getName();
                    intVars.push_back(varName);
                    Value * varInst = &(*it);
                    nameToValue.emplace(varName, varInst);
                    valueToName.emplace(varInst, varName);
                    // errs() << "added the global var\n";
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
        return loadsToAllStores;
    }

    #ifdef ALIAS
    bool mayAliasWithGlobal(Value *ptrType, AliasAnalysis *AA) {
        for (auto itr: globalVars) {
            if (!AA->isNoAlias(ptrType, nameToValue[itr])) {
                return true;
            }
        }
        return false;
    }

    unordered_set<Value*> getGlobalAliases(Value *ptrType, AliasAnalysis *AA) {
        unordered_set<Value*> aliases;
        for (auto itr: globalVars) {
            if (!AA->isNoAlias(ptrType, nameToValue[itr])) {
                aliases.insert(nameToValue[itr]);
            }
        }
        return aliases;
    }
    #endif

    void addInstToMaps(Instruction* inst, unsigned short int tid, unsigned int* instId) {
        auto instNum = make_pair(tid,*instId);
        instToNum.emplace(inst, instNum);
        numToInst.emplace(instNum, inst);
        (*instId)++;
    }

    void initThreadDetails(Module &M) {
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads;
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores;

        // funcToTCreate: func -> tcreate of func
        map<Function*, Instruction*> funcToTCreate;
        // funcToTJoin: func -> tjoin of func
        map<Function*, Instruction*> funcToTJoin;

        //find main function
        Function *mainF = getMainFunction(M);

        threads.push_back(mainF);

        queue<Function*> funcQ;
        unordered_set<Function*> funcSet;
        funcQ.push(mainF);
        funcSet.insert(mainF);
       
        int ssaVarCounter = 0;
        unsigned short int tid = 0;                     // main always have tid 0

        while(!funcQ.empty())
        {
            map<string, Instruction*> lastWritesCurInst;
            Function *func = funcQ.front();
            funcQ.pop();
            vector<string> funcVars;
            
            // errs() << "----analyzing funtion: " << func->getName() << "\n";
            unordered_map<string, unordered_set<Instruction*>> varToStores;
            unordered_map<Instruction*, string> varToLoads;
            #ifdef ALIAS
            AliasAnalysis *AA = &getAnalysis<AAResultsWrapperPass>(*func).getAAResults();
            aliasAnalyses[func]=AA;
            #endif

            unsigned int instId = 0;

            queue<BasicBlock*> basicBlockQ;
            unordered_set<BasicBlock*> basicBlockSet;
            basicBlockQ.push(&*func->begin());
            basicBlockSet.insert(&*func->begin());
            
            while (!basicBlockQ.empty()) {
                BasicBlock* BB = basicBlockQ.front();
                basicBlockQ.pop();

                // errs() << "\nchecking basic block ";
                // BB->print(errs());

                Instruction *lastGlobalInst=nullptr;
                map<string, Instruction*> lastGlobalOfVar;

                #ifdef NOTRA
                map<string, StoreInst*> lastRelWrite;
                #endif

                for(auto I = BB->begin(); I != BB->end(); I++)       //iterator of BasicBlock over Instruction
                {
                    // if (!minimalZ3) 
                        addInstToMaps(&*I, tid, &instId);
                    if (CallInst *call = dyn_cast<CallInst>(I)) {
                        if(!call->getCalledFunction()->getName().compare("pthread_create")) {
                            if (Function* newThread = dyn_cast<Function> (call->getArgOperand(2)))
                            {  
                                auto inSet = funcSet.insert(newThread);
                                if (inSet.second) {
                                    funcQ.push(newThread);
                                    threads.push_back(newThread); 	
                                    // need to add dominates rules
                                    if (minimalZ3) {
                                        vector<Instruction*> lastGlobalInstBeforeCall = getLastGlobalInst(call);
                                        vector<Instruction*> nextGlobalInstAfterCall  = getNextGlobalInst(call->getNextNode());
                                        vector<Instruction*> firstGlobalInstInCalled  = getNextGlobalInst(&*(newThread->begin()->begin()));
                                        // lastGlobalInstBeforeCall (or firstGlobalInstInCalled) == nullptr means there 
                                        // no global instr before thread create in current function (or in newly created thread)
                                        for (auto inst:lastGlobalInstBeforeCall)
                                            zHelper.addSB(inst, call);
                                        for (auto inst:nextGlobalInstAfterCall) 
                                            zHelper.addSB(call, inst);
                                        for (auto inst:firstGlobalInstInCalled)
                                            zHelper.addSB(call, inst);
                                        // if (lastGlobalInst)
                                        //     zHelper.addSB(lastGlobalInst, call);
                                    }
                                    else {
                                        funcToTCreate.emplace(newThread, call);
                                    }
                                }

                            }
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
                            Function* joineeThread = findFunctionFromPthreadJoin(call);
                            if (minimalZ3) {
                                vector<Instruction*> lastGlobalInstBeforeCall = getLastGlobalInst(call);
                                vector<Instruction*> nextGlobalInstAfterCall  = getNextGlobalInst(call->getNextNode());
                                vector<Instruction*> lastGlobalInstInCalled = getLastInstOfPthreadJoin(joineeThread);
                                for (auto inst:nextGlobalInstAfterCall)
                                    zHelper.addSB(call, inst);
                                for (auto inst:lastGlobalInstBeforeCall)
                                    zHelper.addSB(inst, call);
                                for (auto inst: lastGlobalInstInCalled) {
                                    zHelper.addSB(inst, call);
                                }
                                // if (lastGlobalInst) {
                                //     if (minimalZ3) zHelper.addSB(lastGlobalInst, call);
                                // }
                            }
                            else funcToTJoin.emplace(joineeThread, call);
                        }
                        else {
                            if (!noPrint) {
                                errs() << "unknown function call:\n";
                                I->print(errs());
                                errs() <<"\n";
                            }
                        }
                    }
                    else if (StoreInst *storeInst = dyn_cast<StoreInst>(I)) {
                        Value* destVar = storeInst->getPointerOperand();
                        // errs() << "** Possible global alias of: ";
                        // printValue(destVar);
                        // errs() << "from Inst: ";
                        // printValue(storeInst);
                        #ifdef ALIAS
                        if (storeInst->isAtomic()) {
                            for (auto alias: getGlobalAliases(destVar, AA)) {
                                // printValue(alias);
                                string destVarName = getNameFromValue(alias);
                                varToStores[destVarName].insert(storeInst);
                                // errs() << "****adding store instr for: ";
                                // printValue(storeInst);
                                // zHelper.addStoreInstr(storeInst);
                                lastGlobalOfVar[destVarName] = storeInst;
                                lastWritesCurInst[destVarName] = storeInst;
                            }
                            if (lastGlobalInst) {
                                if (minimalZ3) zHelper.addSB(lastGlobalInst, storeInst);
                            } 
                            lastGlobalInst = storeInst;
                        }
                        #else
                        // Not required after alias analysis
                        if(GEPOperator *gepOp = dyn_cast<GEPOperator>(destVar)){
                            destVar = gepOp->getPointerOperand();
                        }
                        if (dyn_cast<GlobalVariable>(destVar)) {
                            string destVarName = getNameFromValue(destVar);
                            varToStores[destVarName].insert(storeInst);
                            #ifdef NOTRA
                            if (storeInst->getOrdering() == llvm::AtomicOrdering::Release ||
                                storeInst->getOrdering() == llvm::AtomicOrdering::AcquireRelease ||
                                storeInst->getOrdering() == llvm::AtomicOrdering::SequentiallyConsistent) {
                                lastRelWrite[destVarName] = storeInst;
                            }
                            else if (lastRelWrite[destVarName]) {
                                prevRelWriteOfSameVar[storeInst] = lastRelWrite[destVarName];
                            }
                            #endif
                            // errs() << "****adding store instr for: ";
                            // printValue(storeInst);
                            // zHelper.addStoreInstr(storeInst);
                            if (minimalZ3) {
                                vector<Instruction*> lastGlobalInstBeforeCall = getLastGlobalInst(storeInst);
                                for (auto inst:lastGlobalInstBeforeCall)
                                    zHelper.addSB(inst, storeInst);
                            }
                            lastGlobalOfVar[destVarName] = storeInst;
                            lastGlobalInst = storeInst;
                            lastWritesCurInst[destVarName] = storeInst;
                        }
                        #endif
                    }
                    else if (AtomicRMWInst *rmwInst = dyn_cast<AtomicRMWInst>(I)) {
                        Value* destVar = rmwInst->getPointerOperand();
                        // TODO: If rmw is performing operation with another variable,
                        // Require value variable for RMW with variables.
                        // A read of the value variable should also be considered
                        
                        // load part of RMW inst
                        Instruction *inst = dyn_cast<Instruction>(I);
                        string varName = "var" + to_string(ssaVarCounter);
                        ssaVarCounter++;
                        nameToValue.emplace(varName, inst);
                        valueToName.emplace(inst, varName);
                        funcVars.push_back(varName);

                        #ifdef ALIAS
                        if (rmwInst->isAtomic()) {
                            for (auto alias: getGlobalAliases(destVar, AA)) {
                                string destVarName = getNameFromValue(destVar);
                                // store part of RMWInst
                                varToStores[destVarName].insert(rmwInst);
                                lastWritesCurInst[destVarName] = rmwInst;
                                // load part of RMWInst
                                varToLoads[rmwInst].insert(destVarName);
                                lastGlobalOfVar[destVarName] = rmwInst;
                            }
                            if (lastGlobalInst) {
                                if (minimalZ3) zHelper.addSB(lastGlobalInst, rmwInst);
                            }
                            lastGlobalInst = rmwInst;
                        }
                        #else
                        // Not required after alias analysis
                        if(GEPOperator *gepOp = dyn_cast<GEPOperator>(destVar)){
                            destVar = gepOp->getPointerOperand();
                        }
                        if (dyn_cast<GlobalVariable>(destVar)) {
                            if (minimalZ3) {
                                vector<Instruction*> lastGlobalInstBeforeCall = getLastGlobalInst(rmwInst);
                                for (auto inst:lastGlobalInstBeforeCall) {
                                    zHelper.addSB(inst, rmwInst);
                                }
                            }
                            string destVarName = getNameFromValue(destVar);
                            // store part of RMWInst
                            varToStores[destVarName].insert(rmwInst);
                            lastWritesCurInst[destVarName] = rmwInst;
                            // load part of RMWInst
                            varToLoads[rmwInst]=destVarName;

                            lastGlobalOfVar[destVarName] = rmwInst;
                            lastGlobalInst = rmwInst;
                        }
                        #endif
                    }
                    else {
                        Instruction *inst = dyn_cast<Instruction>(I);
                        string varName = "var" + to_string(ssaVarCounter);
                        ssaVarCounter++;
                        nameToValue.emplace(varName, inst);
                        valueToName.emplace(inst, varName);
                        funcVars.push_back(varName);

                        if (LoadInst *loadInst = dyn_cast<LoadInst>(I)) {
                            Value* fromVar = loadInst->getOperand(0);
                            #ifdef ALIAS
                            if (loadInst->isAtomic()) {
                                // errs() << "** Possible global alias of: ";
                                // printValue(fromVar);
                                // errs() << "from Inst: ";
                                // printValue(inst);
                                for (auto alias: getGlobalAliases(fromVar, AA)) {
                                    printValue(alias);
                                    string fromVarName = getNameFromValue(alias);
                                    varToLoads[loadInst].insert(fromVarName);
                                    lastGlobalOfVar[fromVarName] = loadInst;
                                }
                                if (lastGlobalInst) {
                                    if (minimalZ3) zHelper.addSB(lastGlobalInst, loadInst);
                                }
                                lastGlobalInst = loadInst;
                            }
                            #else
                            if (GEPOperator *gepOp = dyn_cast<GEPOperator>(fromVar)){
                                fromVar = gepOp->getPointerOperand();
                            }
                            string fromVarName = getNameFromValue(fromVar);
                            if (dyn_cast<GlobalVariable>(fromVar)) {
                                varToLoads[loadInst]=fromVarName;
                                // errs() << "****adding load instr for: ";
                                // printValue(loadInst);
                                // zHelper.addLoadInstr(loadInst);
                                if (minimalZ3) {
                                    vector<Instruction*> lastGlobalInstBeforeCall = getLastGlobalInst(loadInst);
                                    for (auto inst:lastGlobalInstBeforeCall)
                                        zHelper.addSB(inst, loadInst);
                                }
                                // else no global operation yet. Add MHB with init
                                lastGlobalOfVar[fromVarName] = loadInst;
                                lastGlobalInst = loadInst;
                            }
                            #endif
                        }
                    }
                    lastWrites.emplace(make_pair(&(*I),lastWritesCurInst));
                }

            
            
            for (auto succBB: successors(BB)) {
                if (basicBlockSet.insert(succBB).second)
                    basicBlockQ.push(succBB);
                }
            }
                // }
            // Save loads stores function wise
            allStores.emplace(func, varToStores);
            allLoads.emplace(func, varToLoads);
            
            // errs() << "Loads of function " << func->getName() << "\n";
            // for (auto it=varToLoads.begin(); it!=varToLoads.end(); ++it)
            //     printValue(it->first);

            // errs() << "Z3 after func " << func->getName() << ":\n";
            // errs() << zHelper.toString();

            Environment curFuncEnv;
            curFuncEnv.init(globalVars, funcVars);
            funcInitEnv[func] = make_pair(true,curFuncEnv);
            tid++;
        }

        // errs() << "\nAll Loads:\n";
        // for(auto it1=allLoads.begin(); it1!=allLoads.end(); ++it1) {
        //     errs() << "Function " << it1->first->getName() << "\n";
        //     auto loadsOfFun = it1->second;
        //     for (auto it2=loadsOfFun.begin(); it2!=loadsOfFun.end(); ++it2) {
        //         printValue(it2->first);
        //     }
        // }

        getFeasibleInterferences(allLoads, allStores, &funcToTCreate, &funcToTJoin);
    }

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unordered_map <Function*, unordered_map<Instruction*, Environment>> programStateCurItr;
        bool isFixedPointReached = false;

        // map <Function*, map

        while (!isFixedPointReached) {
            programState = programStateCurItr;

            if (!noPrint) {
                errs() << "_________________________________________________\n";
                errs() << "Iteration: " << iterations << "\n";
                // printProgramState();
            }
            // errs() << "Iteration: " << iterations << "\n";
            // #pragma omp paralle for shared(feasibleInterfences,programStateCurItr) private(funcItr) num_threads(threads.size()) chuncksize(1)
            for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
                Function *curFunc = (*funcItr);
                if (!noPrint) {
                    errs() << "\n******** DEBUG: Analyzing thread " << curFunc->getName() << "********\n";
                }
                
                // find feasible interfernce for current function
                vector <map <Instruction*, Instruction*>> curFuncInterfs;
                unordered_map<Instruction*, Environment> newFuncEnv;
                
                auto searchCurFuncInitEnv = funcInitEnv.find(curFunc);
                assert(searchCurFuncInitEnv!=funcInitEnv.end() && "Intial Environment of the function not found");
                
                auto searchInterf = feasibleInterfences.find(curFunc);           
                if (searchInterf != feasibleInterfences.end()) {
                    curFuncInterfs = (searchInterf->second);
                    programStateCurItr[curFunc].clear();
                } else {
                    if (!noPrint) errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        map <Instruction*, Instruction*> interf;
                        newFuncEnv = analyzeThread(curFunc, interf, searchCurFuncInitEnv->second.second, 
                                            searchCurFuncInitEnv->second.first);
                        programStateCurItr.emplace(curFunc, newFuncEnv);
                    }
                    else searchCurFuncInitEnv->second.first = false;
                }
                
                if (searchCurFuncInitEnv->second.first) { 
                    // init env has changed. analyze for all interfs
                    // errs() << "Number of interf= " << curFuncInterfs.size();
                    // analyze the Thread for each interference
                    // #pragma omp parallel for shared(programStateCurItr) private(newFuncEnv) chunksize(500)
                    for (auto interfItr=curFuncInterfs.begin(); interfItr!=curFuncInterfs.end(); ++interfItr){
                        // errs() << "\n***Forinterf\n";

                        newFuncEnv = analyzeThread(*funcItr, *interfItr, searchCurFuncInitEnv->second.second, 
                                            searchCurFuncInitEnv->second.first);

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
                else {
                    // init env has not changed. analyze only for new interfs
                    
                }
            }
            
            // errs() << "------ Program state at iteration " << iterations << ": -----\n";
            // for (auto it: programStateCurItr) {
            //     for (auto it2: it.second) {
            //         printValue(it2.first);
            //         it2.second.printEnvironment();
            //     }
            // }

            // isFixedPointReached = true;
            // TODO: fixed point is reached when no new interf for any func found and the init env has not changed
            isFixedPointReached = isFixedPoint(programStateCurItr);
            iterations++;
        }
        if (!noPrint) {
            errs() << "_________________________________________________\n";
            errs() << "Fixed point reached in " << iterations << " iterations\n";
            errs() << "Final domain:\n";
            printProgramState();
        }
    }

    unordered_map<Instruction*, Environment> analyzeThread (Function *F, map<Instruction*, Instruction*> interf,
        Environment initEnv, bool hasFuncInitEnvChanged
    ) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, Environment> curFuncEnv;
        curFuncEnv[&(*(F->begin()->begin()))] = funcInitEnv[F].second;
        // errs() << "CurDuncEnv before checking preds:\n";
        // printInstToEnvMap(curFuncEnv);

        for(auto bbItr=F->begin(); bbItr!=F->end(); ++bbItr){
            BasicBlock *currentBB = &(*bbItr);

            Environment predEnv = funcInitEnv[F].second;
            
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

            // errs() << "CurFuncEnv before calling analyzeBB:\n";
            // printInstToEnvMap(curFuncEnv);
            analyzeBasicBlock(currentBB, curFuncEnv, interf);
        }

        return curFuncEnv;
    }

    Environment analyzeBasicBlock (BasicBlock *B, 
        unordered_map <Instruction*, Environment> &curFuncEnv,
        map <Instruction*, Instruction*> interf
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
                    // if (!curEnv.isUnreachable()) {
                    //     errs() << "__________________________________________________\n";
                    //     errs() << "ERROR: Assertion failed\n";
                    //     if (!noPrint) {
                    //         printValue(callInst);
                    //         curEnv.printEnvironment();
                    //     }
                    //     exit(0);
                    // }
                }
                else if(!callInst->getCalledFunction()->getName().compare("pthread_create")) {
                    if (Function* newThread = dyn_cast<Function> (callInst->getArgOperand(2))) {
                        // Change the funcInitEnv of newThread
                        checkThreadCreate(callInst, curEnv);
                    }
                }
                else if (!callInst->getCalledFunction()->getName().compare("pthread_join")) {
                    // errs() << "Env before thread join:\n";
                    // curEnv.printEnvironment();
                    curEnv = checkThreadJoin(callInst, curEnv);
                    // errs() << "Env after thread join:\n";
                    // curEnv.printEnvironment();
                }
            }
            else if(AtomicRMWInst *rmwInst = dyn_cast<AtomicRMWInst>(instItr)) {
                // errs() << "initial env:\n";
                // curEnv.printEnvironment();
                curEnv = checkRMWInst(rmwInst, curEnv, interf);
            }
            // CMPXCHG
            else {
                
            }

            curFuncEnv[currentInst] = curEnv;
            predEnv.copyEnvironment(curEnv);
            if (!noPrint) curEnv.printEnvironment();
        }
           
        return curEnv;
    }

    string getNameFromValue (Value *val) {
        // if (val->getType()->isPointerTy()) {
        //     errs() << "Pointer type. Skip\n";
        //     return "";
        // }
        if(GEPOperator *gepOp = dyn_cast<GEPOperator>(val)){
           val = gepOp->getPointerOperand();
        }
        auto searchName = valueToName.find(val);
        if (searchName == valueToName.end()) {
            errs() << "ERROR: Instrution not found in Instruction to Name map\n";
            printValue(val);
            // exit(0);
            return "";
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
            printValue(fromVar1);
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
            printValue(fromVar2);
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
        #ifdef ALIAS
        // if (storeInst->isAtomic() && getGlobalAliases(destVar, aliasAnalyses[storeInst->getFunction()]).size()>1) {
        //     // TODO: since the instruction might be storing to any of the possible aliases, 
        //     // the domain of source var should be joined with domain of all possible 
        //     // aliases of dest var
        //     return curEnv;
        // }
        #endif

        string destVarName = getNameFromValue(destVar);

        #ifdef NOTRA
        auto ord = storeInst->getOrdering();
        if (ord==llvm::AtomicOrdering::Release || 
            ord==llvm::AtomicOrdering::SequentiallyConsistent ||
            ord==llvm::AtomicOrdering::AcquireRelease) {
        #endif
        // if (curEnv.getRelHead(destVarName) == nullptr)
        //     curEnv.setRelHead(destVarName, storeInst);
        // curEnv.changeRelHeadIfNull(destVarName, storeInst);
        
        // if(useMOPO) {
            // errs() << "appending " << storeInst << " to " << destVarName << "\n";
        curEnv.performStoreOp(storeInst, destVarName, zHelper);
        // }
        
        #ifdef NOTRA
        }
        else {
            curEnv.changeRelHeadToNull(destVarName, storeInst);
        }
        #endif

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
        map<Instruction*, Instruction*> interf
    ) {
        Value* fromVar = unaryInst->getOperand(0);
        string fromVarName = getNameFromValue(fromVar);
        string destVarName = getNameFromValue(unaryInst);
        
        /* if (LoadInst *loadInst = dyn_cast<LoadInst>(unaryInst)) {
            if (loadInst->isAtomic()) {
                if(GEPOperator *gepOp = dyn_cast<GEPOperator>(fromVar)){
                    fromVar = gepOp->getPointerOperand();
                }           
                if (dyn_cast<GlobalVariable>(fromVar)) {
                    // errs() << "Load of global\n";
                    // errs() << "Env before:";
                    // curEnv.printEnvironment();
                    curEnv = applyInterfToLoad(unaryInst, curEnv, interf, fromVarName);
                }
            }
            curEnv.performUnaryOp(LOAD, destVarName.c_str(), fromVarName.c_str());
        } */
        switch (unaryInst->getOpcode()) {
            case Instruction::Load:
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
        
        curEnv.performUnaryOp(LOAD, destVarName.c_str(), fromVarName.c_str());

        return curEnv;
    }

    void checkThreadCreate(CallInst *callInst, Environment curEnv) {
        // TODO: need to call this oper only if curEnv has changed and set changed to false
        // Carry the environment of current thread to the newly created thread. 
        // Need to copy only globals, discard locals.
        if (Function* calledFunc = dyn_cast<Function> (callInst->getArgOperand(2))) {
            funcInitEnv[calledFunc].second.copyOnVars(curEnv, globalVars);
            funcInitEnv[calledFunc].first = true;
        }
    }

    Environment checkThreadJoin(CallInst *callInst, Environment curEnv) {
        // Join the environments of joinee thread with this thread. 
        // Need to copy only globals, discard locals.
        Function *calledFunc = findFunctionFromPthreadJoin(callInst);
        for (auto bbItr=calledFunc->begin(); bbItr!=calledFunc->end(); ++bbItr) {
            for (auto instItr=bbItr->begin(); instItr!=bbItr->end(); ++instItr) {
                if (ReturnInst *retInst = dyn_cast<ReturnInst>(instItr)) {
                    // errs() << "pthread join with: ";
                    // printValue(retInst);
                    curEnv.joinOnVars(programState[calledFunc][retInst], globalVars, 
                        &lastWrites, retInst, callInst, zHelper);
                }
            }
        }
        return curEnv;
    }

    Environment checkRMWInst(
        AtomicRMWInst *rmwInst, 
        Environment curEnv, 
        map<Instruction*, Instruction*> interf
    ) { 
        Value *pointerVar = rmwInst->getPointerOperand();
        string pointerVarName = getNameFromValue(pointerVar);
        string destVarName = getNameFromValue(rmwInst);
        
        // what type of RMWInst
        operation oper;
        switch(rmwInst->getOperation()) {
            case AtomicRMWInst::BinOp::Add:
                oper = ADD;
                break;
            case AtomicRMWInst::BinOp::Sub:
                oper = SUB;
                break;
            default:
                errs() << "WARNING: unsupported operation " << rmwInst->getOpcodeName() << "\n";
                printValue(rmwInst);
                return curEnv;
        }

        // apply the interf on load of global for RMW
        if (GEPOperator *gepOp = dyn_cast<GEPOperator>(pointerVar)) {
            pointerVar = gepOp->getPointerOperand();
        }
        if (dyn_cast<GlobalVariable>(pointerVar)) {
            curEnv = applyInterfToLoad(rmwInst, curEnv, interf, pointerVarName);
        }
        // errs() << "After applyInterf:\n";
        // curEnv.printEnvironment();

        // the old value of global is returned
        curEnv.performUnaryOp(LOAD, destVarName.c_str(), pointerVarName.c_str());

        // errs() << "After assigning to the destVar:\n";
        // curEnv.printEnvironment();

        // find the argument to perform RMW with and 
        // update the new value of global variable
        Value *withVar = rmwInst->getValOperand();
        if (ConstantInt *constWithVar = dyn_cast<ConstantInt>(withVar)) {
            int constIntWithVar= constWithVar->getValue().getSExtValue();
            curEnv.performBinaryOp(oper, pointerVarName, pointerVarName, constIntWithVar);
        }
        else {
            string withVarName = getNameFromValue(withVar);
            curEnv.performBinaryOp(oper, pointerVarName, pointerVarName, withVarName);
        }

        // errs() << "After performing binOp:\n";
        // curEnv.printEnvironment();

        // append the current instruction in POMO
        curEnv.performStoreOp(rmwInst, pointerVarName, zHelper);

        // errs() << "After appending store:\n";
        // curEnv.printEnvironment();
        return curEnv;
    }

    Environment applyInterfToLoad(
        Instruction* loadInst, 
        Environment curEnv, 
        map<Instruction*, Instruction*> interf,
        string varName
    ) {
        // errs() << "Applying interf\n";
        // find interfering instruction
        auto searchInterf = interf.find(loadInst);
        if (searchInterf == interf.end()) {
            errs() << "ERROR: Interfernce for the load instrction not found\n";
            printValue(loadInst);
            return curEnv;
        }
        Instruction *interfInst = searchInterf->second;
        
        // if interfernce is from some other thread
        if (interfInst && interfInst != loadInst->getPrevNode()) {
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
                    
                    bool isRelSeq = true;       // Since the memory model is RA
                    #ifdef NOTRA
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
                    #endif
                    curEnv.applyInterference(varName, interfEnv, zHelper, interfInst, loadInst, &lastWrites);
                }
            }
        }
        return curEnv;
    }

    /// Compute all possible interferences (feasibile or infeasible)
    unordered_map<Function*, vector< map<Instruction*, Instruction*>>> getAllInterferences (
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores
    ){
        unordered_map<Function*, vector< map<Instruction*, Instruction*>>> allInterfs;
        // printLoadsToAllStores(loadsToAllStores);

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
            
            map<Instruction*, Instruction*> curInterf;
            
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
                while (k>=0 && allItr[k] == allLS[loads[k]].end()) {
                    allItr[k] = allLS[loads[k]].begin();
                    k--;
                    if (k>=0) allItr[k]++;
                }
            }
        }
        
        return allInterfs;
    }

    /// Checks all possible interfernces for feasibility one by one.
    void getFeasibleInterferences (
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores,
        const map<Function*, Instruction*> *funcToTCreate,
        const map<Function*, Instruction*> *funcToTJoin
    ){
        unordered_map<Function*, vector< map<Instruction*, Instruction*>>> allInterfs;
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores;
        // Make all permutations
        loadsToAllStores = getLoadsToAllStoresMap(allLoads, allStores);
        allInterfs = getAllInterferences(loadsToAllStores);

        // errs() << "# of total interference:\n";
        // for (auto it: allInterfs) {
        //     errs() << it.first->getName() << " : " << it.second.size() << "\n";
        // }

        // Check feasibility of permutations and save them in feasibleInterfences
        // Older code. Not required for RA and Z3Minimal
        #ifdef NOTRA
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
        #endif

        // errs() << "#interfs: " << allInterfs.size() << "\n";

        // #pragma omp parallel num_threads(omp_get_num_procs()*2)
        // #pragma omp single 
        // {
        #pragma omp parallel for shared(feasibleInterfences, minimalZ3,funcToTCreate,funcToTJoin) num_threads(allInterfs.size())
        for (auto funcItr=allInterfs.begin(); funcItr!=allInterfs.end(); ++funcItr) {
            vector< map<Instruction*, Instruction*>> curFuncInterfs;
            for (auto interfItr=funcItr->second.begin(); interfItr!=funcItr->second.end(); ++interfItr) {
                auto interfs = *interfItr;
                // #pragma omp task private(interfs) shared(curFuncInterfs)
                // {
                if (minimalZ3) {
                    if(isFeasibleRA(*interfItr))
                        curFuncInterfs.push_back(*interfItr);
                }
                else {
                    // double start_time = omp_get_wtime();
                    bool isFeasible = isFeasibleRAWithoutZ3(*interfItr, funcToTCreate, funcToTJoin);
                    // errs() << "time: " << (omp_get_wtime() - start_time) << "\n";
                    if (isFeasible) curFuncInterfs.push_back(*interfItr);
                }   
                // }
            }
            // #pragma omp taskwait
            feasibleInterfences[funcItr->first] = curFuncInterfs;
        }
        // }
    }

    /// Older function. Used with UseZ3 flag. 
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

    /** This function checks the feasibility of an interference combination 
     * under RA memory model 
     */
    bool isFeasibleRA(map<Instruction*, Instruction*> interfs) {
        for (auto lsPair=interfs.begin(); lsPair!=interfs.end(); ++lsPair) {
            if (lsPair->second == nullptr)
                continue;
            if (zHelper.querySB(lsPair->first, lsPair->second)) return false;
            for (auto otherLS=interfs.begin(); otherLS!=interfs.end(); ++otherLS) {
                // lsPair: (s --rf--> l), otherLS: (s' --rf--> l')
                if (otherLS == lsPair || otherLS->second==nullptr)
                    continue;
                Instruction *ld = lsPair->first;
                Instruction *st = lsPair->second;
                Instruction *ld_prime = otherLS->first;
                Instruction *st_prime = otherLS->second;
                
                // (l --sb--> l')
                if (zHelper.querySB(ld, ld_prime)) {
                    // (l --sb--> l' && s = s') reading from local context will give the same result
                    if (st == st_prime) return false;
                    else if (zHelper.querySB(st_prime, st)) return false;
                }
            }
        }
        return true;
    }

    bool isFeasibleRAWithoutZ3(map<Instruction*, Instruction*> interfs,
        const map<Function*, Instruction*> *funcToTCreate,
        const map<Function*, Instruction*> *funcToTJoin
    ) {
        for (auto lsPair=interfs.begin(); lsPair!=interfs.end(); ++lsPair) {
            if (lsPair->second == nullptr)
                continue;
            
            if (isSeqBefore(lsPair->first, lsPair->second)) return false;
            if (SBTCreateTJoin(lsPair->first, lsPair->second, funcToTCreate, funcToTJoin)) return false;
            for (auto otherLS=interfs.begin(); otherLS!=interfs.end(); ++otherLS) {
                // lsPair: (s --rf--> l), otherLS: (s' --rf--> l')
                if (otherLS == lsPair || otherLS->second==nullptr)
                    continue;
                Instruction *ld = lsPair->first;
                Instruction *st = lsPair->second;
                Instruction *ld_prime = otherLS->first;
                Instruction *st_prime = otherLS->second;
                // (l --sb--> l')
                if (isSeqBefore(ld, ld_prime)) {
                    // (l --sb--> l' && s = s') reading from local context will give the same result
                    if (st == st_prime) return false;
                    else if (isSeqBefore(st_prime, st)) return false;
                }
            }
        }
        return true;
    }

    #ifdef NOTRA
    /** This function checks feasibility of an interfernce combination
     * under C/C++11 minus SC memory model 
     */
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
    #endif

    
    bool SBTCreateTJoin(Instruction* ld, Instruction* st, 
        const map<Function*, Instruction*> *funcToTCreate,
        const map<Function*, Instruction*> *funcToTJoin
    ) {
        Function* ldFunc = ld->getFunction();
        // lab: tcreate(f) /\ l in f /\ s--sb-->lab ==> s--nrf-->l
        auto searchLdFuncTC = funcToTCreate->find(ldFunc);
        if (searchLdFuncTC != funcToTCreate->end()) {
            if(isSeqBefore(st, searchLdFuncTC->second)) return true;
        }
        else {
            // since no tcreate instruction is found, the function name must be main
            assert(ldFunc->getName()=="main");
        }

        // lab: tjoin(f) /\ l in f /\ lab--sb-->s ==> s--nrf-->l
        auto searchldFuncTJ = funcToTJoin->find(ldFunc);
        if (searchldFuncTJ != funcToTJoin->end()) {
            if(isSeqBefore(searchldFuncTJ->second, st)) return true;
        }
        else {
            // since no tjoin instruction is found, the function name must be main
            assert(ldFunc->getName()=="main");
        }

        Function* stFunc = st->getFunction();
        // lab: tcreate(f) /\ s in f /\ l--sb-->lab ==> s--nrf-->l
        auto searchStFucnTC = funcToTCreate->find(stFunc);
        if (searchStFucnTC != funcToTCreate->end()) {
            if(isSeqBefore(ld, searchStFucnTC->second)) return true;
        }
        else {
            // since no tcreate instruction is found, the function name must be main
            assert(stFunc->getName()=="main");
        }

        // lab: tjoin(f) /\ s in f /\ lab--sb-->l ==> s--nrf-->l
        auto searchStFuncTJ = funcToTJoin->find(stFunc);
        if (searchStFuncTJ != funcToTJoin->end()) {
            if(isSeqBefore(searchStFuncTJ->second, ld)) return true;
        }
        else {
            // since no tjoin instruction is found, the function name must be main
            assert(stFunc->getName()=="main");
        }
        return false;
    }

    unordered_map<Instruction*, Environment> joinEnvByInstruction (
        unordered_map<Instruction*, Environment> instrToEnvOld,
        unordered_map<Instruction*, Environment> instrToEnvNew
    ) {
        // new = old join new
        // #pragma omp parallel for shared(instrToEnvNew)
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
                            errs() << "Assertion failed:\n";
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
        errs() << "# Failed Asserts: " << num_errors << "\n";
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
        // auto funIt = funcInitEnv.begin();
        // Function *fun1 = funIt->first;
        // Environment fun1Env = funIt->second;
        // fun1Env.performUnaryOp(STORE, "x", 1);
        // fun1Env.performUnaryOp(STORE, "y", 10);
        // funIt++;
        // Function *fun2 = funIt->first;
        // Environment fun2Env = funIt->second;
        // errs() << "Interf from domain:\n";
        // fun1Env.printEnvironment();
        // errs() << "To domain:\n";
        // fun2Env.printEnvironment();
        // fun2Env.applyInterference("x", fun1Env, zHelper);
        // errs() << "After applying:\n";
        // fun2Env.printEnvironment();
    }

    void testPO() {
        /* Test using test32 */
        // PartialOrder po, po2;
        // auto p1=prevRelWriteOfSameVar.begin()->first;
        // auto p2=prevRelWriteOfSameVar.begin()->first;
        // auto p3=prevRelWriteOfSameVar.begin()->first;
        // bool first = true, append=true;
        // for (auto it=prevRelWriteOfSameVar.begin(); it!=prevRelWriteOfSameVar.end(); it++) {
        //     printValue(it->first);
        //     printValue(it->second);
        //     errs() << "\n";
        //     if (it->first!=nullptr && it->second!=nullptr) {
        //         errs() << "Adding PO between "; printValue(it->first); printValue(it->second);
        //         po.addOrder(zHelper, it->first, it->second);
        //         first ? (p1 = it->second) : (first=false, p1= it->first);
        //         // break;
        //     } 
        //     else if (!append) {p3 = ((it->first!=nullptr)?(it->first):(it->second)); append=false;}
        // }
        // errs() << po.toString() << "\n"; 

        // errs() << "second po\n";
        // errs() << p1 << " order with " << p2 << ": " << po2.addOrder(zHelper, p1,p2) << "\n";
        // errs() << po2.toString() << "\n";    

        // errs() << p1 << " isOrderedBefore " << p2 << ": " << po.isOrderedBefore(p1,p2) << "\n";
        // errs() << p2 << " isOrderedBefore " << p1 << ": " << po.isOrderedBefore(p2,p1) << "\n";
        // errs() << p1 << " isOrderedBefore " << p2 << ": " << po2.isOrderedBefore(p1,p2) << "\n";
        // errs() << p2 << " isOrderedBefore " << p1 << ": " << po2.isOrderedBefore(p2,p1) << "\n";

        // errs() << "\npo Join po2\n";
        // errs() << po.join(zHelper, po2) << "\n";
        // errs() << po.toString() << "\n";

        // errs() << p1 << " isOrderedBefore " << p2 << ": " << po.isOrderedBefore(p1,p2) << "\n";
        // errs() << p2 << " isOrderedBefore " << p1 << ": " << po.isOrderedBefore(p2,p1) << "\n";
        // errs() << p1 << " isOrderedBefore " << p2 << ": " << po2.isOrderedBefore(p1,p2) << "\n";
        // errs() << p2 << " isOrderedBefore " << p1 << ": " << po2.isOrderedBefore(p2,p1) << "\n";

        // errs() << "append " << p2 << ": " << po.append(zHelper, p2) << "\n";
        // errs() << po.toString() << "\n";

        // errs() << "removing " << p1 << "\n";
        // po.remove(p1);
        // errs() << po.toString() << "\n";

        // errs() << "Append " << p1 << "again\n";
        // po.append(zHelper, p1);
        // errs() << po.toString() << "\n";
        
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

    void printNumFeasibleInterf () {
        errs() << "# of feasible interference:\n";
        for (auto it: feasibleInterfences) {
            errs() << it.first->getName() << " : " << it.second.size() << "\n";
        }
    }

    void printFeasibleInterf() {
        errs() << "\nFeasible Interfs\n";
        for (auto it1=feasibleInterfences.begin(); it1!=feasibleInterfences.end(); ++it1) {
            errs() << "Interfs for function: " << it1->first->getName() << "\n";
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

    bool isGlobalRMW(Instruction *inst) {
        if(AtomicRMWInst *rmwInst = dyn_cast<AtomicRMWInst>(inst)) {
            Value* destVar = rmwInst->getPointerOperand();
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

    Instruction* getLastGlobalInstInBB(Instruction *inst) {
        Instruction *prevInst = inst->getPrevNode();
        while ( prevInst                    &&
                ! isGlobalLoad(prevInst)    &&
                ! isGlobalStore(prevInst)   &&
                ! isGlobalRMW(prevInst)     &&
                ! isPthreadCreate(prevInst) &&
                ! isPthreadJoin(prevInst)) {
            prevInst = prevInst->getPrevNode();
        } 
        return prevInst;
    }

    vector<Instruction*> getLastGlobalInst(Instruction *inst) {
        vector<Instruction*> lastInst;
        list<Instruction*> findLastOf;
        findLastOf.push_back(inst);

        // loop over all terminating BasicBlocks to find the last global instructions
        while (!findLastOf.empty()) {
            Instruction* inst = findLastOf.front();
            findLastOf.pop_front();
            auto li = getLastGlobalInstInBB(inst);
            if (li) lastInst.push_back(li);
            else {
                for (auto predBB: predecessors(inst->getParent())) findLastOf.push_back(predBB->getTerminator());
            }
        }
        return lastInst;

    }

    Instruction* getNextGlobalInstInBB(Instruction *inst) {
        Instruction *nextInst = inst;
        while ( nextInst                    &&
                ! isGlobalLoad(nextInst)    &&
                ! isGlobalStore(nextInst)   &&
                ! isGlobalRMW(nextInst)     &&
                ! isPthreadCreate(nextInst) &&
                ! isPthreadJoin(nextInst)) {
            nextInst = nextInst->getNextNode ();
        }
        return nextInst;
    }

    vector<Instruction*> getNextGlobalInst(Instruction *inst) {
        vector<Instruction*> nextInst;
        list<Instruction*> findNextOf;
        findNextOf.push_back(inst);

        // loop over all terminating BasicBlocks to find the last global instructions
        while (!findNextOf.empty()) {
            Instruction* inst = findNextOf.front();
            findNextOf.pop_front();
            auto li = getNextGlobalInstInBB(inst);
            if (li) nextInst.push_back(li);
            else {
                for (auto succBB: successors(inst->getParent())) findNextOf.push_back(succBB->getTerminator());
            }
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

    vector<Instruction*> getLastInstOfPthreadJoin(Function *joineeThread) {
        vector<Instruction*> lastInst;
        // Function *func = findFunctionFromPthreadJoin(call);

        // get all return instructions
        list<Instruction*> retInstList;
        for (auto bbItr=joineeThread->begin(); bbItr!=joineeThread->end(); ++bbItr) {
            TerminatorInst *term = bbItr->getTerminator();
            if (ReturnInst *ret = dyn_cast<ReturnInst>(term)) {
                // get the last inst of return instruction and add them to lastInst
                vector<Instruction*> li = getLastGlobalInst(ret);
                copy(li.begin(), li.end(), inserter(lastInst, lastInst.end()));
            }
        }
        return lastInst;
    }

    void printInstMaps() {
        errs() << "\n\n-----------Printing inst to inst num-----------\n";
        for (auto it: instToNum) {
            fprintf(stderr, "%p: ", it.first);
            it.first->print(errs());
            errs() << "\t(" << it.second.first << "," << it.second.second << ")\n";
        }
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

char VerifierPass::ID = 10;
static RegisterPass<VerifierPass> X("verifier", "Abstract Interpretation Verifier Pass", false, true);
