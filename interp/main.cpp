#include "common.h"
#include "domain.h"
#include "analyzer.h"
#include <iterator>
#include <llvm/Support/CommandLine.h>
// #include "interfernce.h"


// Processing command line arguments

// command line argument for domain type
cl::opt<DomainTypes> AbsDomType(cl::desc("Choose abstract domain to be used"),
    cl::values(
        clEnumVal(interval , "use interval domain"),
        clEnumVal(octagon, "use octagon domain")));
cl::opt<PrecisionLevel> Precision(cl::desc("Choose precision level for analysis"),
	cl::values(
		clEnumVal(P0, "level 0: RMW's are treated as any other store operation"),
		clEnumVal(P1, "keep at max K=?? RMW for each therad in PO"),
		clEnumVal(P2, "keep all RMW in PO, consistency check of RMW is limited	to checking that combining them maintains TO among RMW"),
		clEnumVal(P3, "P2 + conssistency check makes sure that interf PO of RMW is ordered after curPO of RMW")
	));
cl::opt<bool> noPrint   ("no-print", cl::desc("Do not print debug output"));
// cl::opt<bool> useMOHead ("useMOHead", cl::desc("Enable interference pruning using Z3 using modification order head based analysis"));
// cl::opt<bool> useMOPO ("useMOPO", cl::desc("Enable interference pruning using Z3 using partial order over modification order based analysis"));
cl::opt<bool> stopOnFail("stop-on-fail", cl::desc("Stop the analysis as soon as assertion is failed"));
cl::opt<bool> eagerPruning("eager-pruning", cl::desc("Eagerly prune infeasible interference combinations"));
cl::opt<bool> noInterfComb   ("no-interf-comb", cl::desc("Use analysis without interference combinations"));
cl::opt<bool> mergeOnVal ("merge-on-val", cl::desc("merge executions when values are same"));



map<llvm::Instruction*, InstNum> instToNum;
map<InstNum, llvm::Instruction*> numToInst;
unordered_map <string, set<llvm::Instruction*>> lockVarToUnlocks;
unordered_set<string> lockVars;

class VerifierPass : public ModulePass {

    typedef EnvironmentPOMO Environment;
    // typedef EnvironmentRelHead Environment;

    vector <Function*> threads;
    unordered_map <Function*, unordered_map<Instruction*, Environment*>> programState;
    unordered_map <Function*, unordered_map<Instruction*, Environment*>> mergeProgramState; 
    // initial environment of the function. A map from Func-->(isChanged, Environment)
    unordered_map <Function*, Environment*> funcInitEnv;
    set<pair<Instruction*, Instruction*>> allLSPairs;
    // map <Function*, forward_list<InterfNode*>> newFeasibleInterfs;
    int maxFeasibleInterfs=0;

    unordered_map <string, Value*> nameToValue;
    unordered_map <Value*, string> valueToName;
    map<Instruction*, map<string, Instruction*>> lastWrites; 
    
    vector<string> globalVars;
    unordered_map<Instruction*, Instruction*> lockToUnlock;

    unsigned int iterations = 0;
    double start_time;

    #ifdef NOTRA
    map<StoreInst*, StoreInst*> prevRelWriteOfSameVar;
    #endif

    #ifdef ALIAS
    map<Function*, AliasAnalysis*> aliasAnalyses;

    void getAnalysisUsage(AnalysisUsage &AU) const override {
        AU.addRequired<AAResultsWrapperPass>();
    }
    #endif
	void compareProgramState(
	    unordered_map <Function*, unordered_map<Instruction*, Environment>> mergeProgramState,
	    unordered_map <Function*, unordered_map<Instruction*, Environment>> unmergeProgramState
	) {
		for (auto itFun=mergeProgramState.begin(); itFun!=mergeProgramState.end(); itFun++) {
			if (itFun->first->getName().find("main")!=llvm::StringRef::npos) continue;
			errs() << "function: " << itFun->first->getName() << "\n";
			auto sFun = unmergeProgramState.find(itFun->first);
			if (sFun == unmergeProgramState.end()) {
				errs() << " not found\n";
				exit(0);
			}
			for (auto itInst=itFun->second.begin(); itInst!=itFun->second.end(); itInst++) {
				errs() << "\nInst: "; printValue(itInst->first);
				auto sInst = sFun->second.find(itInst->first);
				if (sInst == sFun->second.end()) {
					errs() << "Inst not found\n";
				}
				errs() << "with merging:"<< itInst->second.size() << "\n";
				itInst->second.printEnvironment();
				errs() << "without merging"<< sInst->second.size() << "\n";
				sInst->second.printEnvironment();
				errs() << "Comparing\n";
				itInst->second.compareEnv(sInst->second);
			}
		}
	}


    
    bool runOnModule (Module &M) {
        start_time = omp_get_wtime();
        getGlobalIntVars(M);
        // errs() << "#global vars:" << globalVars.size() << "\n";
        // errs() << "#lock vars:" << lockVars.size() << "\n";
        // zHelper.initZ3(globalVars);
        // errs() << "init\n";
        if (noInterfComb) {
            unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> feasibleInterfences;
            initThreadDetails(M, feasibleInterfences);
			// errs() << "Init Done\n";
            // printInstMaps();
            // errs() << "Valriable to instName map:\n";
            // for (auto it: nameToValue) {
            //     errs() << it.first << ":"; printValue(it.second);
            // }
            // errs() << "\n Feasible Interfs:\n";
            // printLoadsToAllStores(feasibleInterfences);
            // countNumFeasibleInterf(feasibleInterfences);
			//errs() << "with merging on val\n";
			//mergeOnVal = true;
			// errs() << "analyzing\n";
            analyzeProgram(M, feasibleInterfences);
			//mergeProgramState = programState;
			//programState.clear();
			//errs() << "without merging\n";
			//iterations=0;
			//mergeOnVal = false;
            //initThreadDetails(M, feasibleInterfences);
			//analyzeProgram(M, feasibleInterfences);
			//compareProgramState(mergeProgramState, programState);
        }
        else  {
            // map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> feasibleInterfences;
            // initThreadDetails(M, feasibleInterfences);
            // analyzeProgram(M, feasibleInterfences);
            // countNumFeasibleInterf(feasibleInterfences);
            // printFeasibleInterf(feasibleInterfences);
        }
        // printInstMaps();
        // testPO();
        // errs() << "analyze\n";
        if (!stopOnFail) checkAssertions();
        else {
            errs() << "___________________________________________________\n";
            errs() << "# Failed Asserts: 0\n";
        }
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

    void getGlobalIntVars(Module &M) {
        for (auto it = M.global_begin(); it != M.global_end(); it++){
            // errs() << "Global var: " << it->getName() << "\tType: ";
            // it->getValueType()->print(errs());
            // errs() << "\n";
            if (it->getValueType()->isIntegerTy() && it->getName()!="__dso_handle") {
                string varName = it->getName();
                globalVars.push_back(varName);
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
                // errs() << "structName:" << it->getValueType()->getStructName() << "\n";
                // errs() << "found structty:"; structTy->print(errs()); errs() << "\n";
                // it->getValueType()->getStructElementType(0)->print(errs()); errs() <<"\n";
                // errs() << "ele type id: " << tmp->getTypeID() << ", FuncType:" << tmp->isFunctionTy() << ", Pointer Ty: " << tmp->isPointerTy() << ", isInt: " << tmp->isIntegerTy() << ", isTokenTy: " << tmp->isTokenTy() << ", isStruct: " << tmp->isStructTy() << "\n";
                if  (!structTy->getName().find("struct.std::atomic")) {
                    string varName = it->getName();
                    globalVars.push_back(varName);
                    Value * varInst = &(*it);
                    nameToValue.emplace(varName, varInst);
                    valueToName.emplace(varInst, varName);
                    // errs() << "added the global var\n";
                }
                else if  (StructType *substruct = dyn_cast<StructType>(structTy->getStructElementType(0))) {
                    if (substruct->getName()=="union.pthread_mutex_t") {
                        string varName = it->getName();
                        lockVars.insert(varName);
                        Value * varInst = &(*it);
                        nameToValue.emplace(varName, varInst);
                        valueToName.emplace(varInst, varName);
                        // errs() << "\nadded the lock var: " << varName << " Type: "<< structTy->getName() << "\n";
						// errs() << "varInst"; printValue(varInst);
                    }
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
        if (!noPrint) {
            errs() << "DEBUG: Total global vars = " << globalVars.size() << "\n";
            errs() << "DEBUG: Tota; lock vars = " << lockVars.size() << "\n";
        }
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
        InstNum inum(tid, *instId);
        instToNum.emplace(inst, inum);
        numToInst.emplace(inum, inst);
        (*instId)++;
    }

    void initThreadDetailsHelper(Module &M, 
        unordered_map<Function*, forward_list<pair<Instruction*, string>>> &allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> &allStores,
        map<Function*, Instruction*> &funcToTCreate,
        map<Function*, Instruction*> &funcToTJoin
    ) {
        //find main function
        Function *mainF = getMainFunction(M);

        threads.push_back(mainF);

        queue<Function*> funcQ;
        unordered_set<Function*> funcSet;
        funcQ.push(mainF);
        funcSet.insert(mainF);
    
        int ssaVarCounter = 0;
        unsigned short int tid = 1;                     // main always have tid 1

        while(!funcQ.empty())
        {
            map<string, Instruction*> lastWritesCurInst;
            Function *func = funcQ.front();
            funcQ.pop();
            vector<string> funcVars;
            
            // errs() << "----init of funtion: " << func->getName() << "\n";
            unordered_map<string, unordered_set<Instruction*>> varToStores;
            forward_list<pair<Instruction*, string>> varToLoads;
            auto varToLoadsIntertPt = varToLoads.before_begin();

            #ifdef ALIAS
            AliasAnalysis *AA = &getAnalysis<AAResultsWrapperPass>(*func).getAAResults();
            aliasAnalyses[func]=AA;
            #endif

            unsigned int instId = 1;

            queue<BasicBlock*> basicBlockQ;
            unordered_set<BasicBlock*> basicBlockSet;
            basicBlockQ.push(&*func->begin());
            basicBlockSet.insert(&*func->begin());
            unordered_set<BasicBlock*> done;
            Instruction *lastLockInst = nullptr;
            
            while (!basicBlockQ.empty()) {
                BasicBlock* BB = basicBlockQ.front();
                basicBlockQ.pop();

                // errs() << "checking if BB is ready for preprocessing: "; printValue(BB);

                bool doAnalyze = true;
                for (auto predBB: predecessors(BB)) {
					if (predBB == BB) {
						errs() << "ERROR: BB is predecessors of itself. Possibly there is a loop in program.\n";
						exit(0);
					}
                    auto searchPred = done.find(predBB);
                    if (searchPred == done.end()) {
                        doAnalyze = false;
                        break;
                    }
                }
                if (!doAnalyze) {
                    basicBlockQ.push(BB);
                    // errs() << "Not Analyzed\n";
                    continue;
                }

                // errs() << "\npreprocessing basic block ";
                // BB->print(errs());

                Instruction *lastGlobalInst=nullptr;
                map<string, Instruction*> lastGlobalOfVar;

                #ifdef NOTRA
                map<string, StoreInst*> lastRelWrite;
                #endif

                for(auto I = BB->begin(); I != BB->end(); I++)       //iterator of BasicBlock over Instruction
                {
                    // if (!minimalZ3) 
                    // errs() << "Inst: "; printValue(&*I); errs() << "\n"; 
                    addInstToMaps(&*I, tid, &instId);
                    // errs() << "added to maps\n";
                    if (CallInst *call = dyn_cast<CallInst>(I)) {
                        if(!call->getCalledFunction()->getName().compare("pthread_create")) {
                            if (Function* newThread = dyn_cast<Function> (call->getArgOperand(2)))
                            {  
                                auto inSet = funcSet.insert(newThread);
                                if (inSet.second) {
                                    funcQ.push(newThread);
                                    threads.push_back(newThread); 	
                                    funcToTCreate.emplace(newThread, call);
                                }

                            }
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            Function* joineeThread = findFunctionFromPthreadJoin(call);
							// errs() << "joinee: " << joineeThread->getName() << " for inst: "; printValue(call); 
                            funcToTJoin.emplace(joineeThread, call);
                        }
                        else if (call->getCalledFunction()->getName().find("unlock")!=llvm::StringRef::npos) {
                            assert(lastLockInst && "Unlock with no matching lock instruction found");
                            lockToUnlock[lastLockInst] = call;
                            lastLockInst = nullptr;

                            // add to lockVarsToUnlock Map
                            if (BitCastOperator *bcOp = dyn_cast<BitCastOperator>(call->getOperand(0))) {
                                lockVarToUnlocks[bcOp->getOperand(0)->getName()].insert(call);
                                // errs() << "Lock Op 0: "; printValue(bcOp->getOperand(0));
                                // errs() << "Name: " << bcOp->getOperand(0)->getName() << "\n";
                            }
                        }
                        else if (call->getCalledFunction()->getName().find("lock")!=llvm::StringRef::npos) {
                            // errs() << "*** Found lock inst: "; printValue(call);
                            lastLockInst = call;
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
                                errs() << "****adding store instr for: ";
                                printValue(storeInst);
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
                            string destVarName = getNameFromValue(destVar);
                            // store part of RMWInst
                            varToStores[destVarName].insert(rmwInst);
                            lastWritesCurInst[destVarName] = rmwInst;
                            // load part of RMWInst
                            varToLoadsIntertPt = varToLoads.insert_after(varToLoadsIntertPt, make_pair(rmwInst, destVarName));
                            lastGlobalOfVar[destVarName] = rmwInst;
                            lastGlobalInst = rmwInst;
                        }
                        #endif
                    }
                    else {
                        if (!dyn_cast<BranchInst>(I)) {
                            Instruction *inst = dyn_cast<Instruction>(I);
                            string varName = "var" + to_string(ssaVarCounter);
                            ssaVarCounter++;
                            nameToValue.emplace(varName, inst);
                            valueToName.emplace(inst, varName);
                            funcVars.push_back(varName);
                        }
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
                                varToLoadsIntertPt = varToLoads.insert_after(varToLoadsIntertPt, make_pair(loadInst, fromVarName));
                                // errs() << "****adding load instr for: ";
                                // printValue(loadInst);
                                // zHelper.addLoadInstr(loadInst);
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
                done.insert(BB);
            }
                // }
            // Save loads stores function wise
            allStores.emplace(func, varToStores);
            allLoads.emplace(func, varToLoads);
            
			// errs() << "ValToName:\n";
			// for (auto it=valueToName.begin(); it!=valueToName.end(); it++) {
			// 	printValue(it->first); errs() << "\t:\t" << it->second << "\n";
			// }

            // errs() << "Loads of function " << func->getName() << "\n";
            // for (auto it=varToLoads.begin(); it!=varToLoads.end(); ++it)
            //     printValue(it->first);

            // errs() << "Z3 after func " << func->getName() << ":\n";
            // errs() << zHelper.toString();
            
            // errs() << "making env\n";
            Environment *curFuncEnv=new Environment();
            // errs() << "init env\n";
            curFuncEnv->init(globalVars, funcVars, lockVars);
            // errs() << "adding to func init env\n";
            // curFuncEnv.printEnvironment();
            funcInitEnv[func] = curFuncEnv;
            tid++;
        }

        // errs() << "\nLock-to-Unlock Inst map:\n";
        // for (auto it=lockToUnlock.begin(); it!=lockToUnlock.end(); it++) {
        //     printValue(it->first); errs () << "\t--->\t"; printValue(it->second);

        // }
        // errs() << "lockVar-to-Unlock Map:\n";
        // for (auto it:lockVarToUnlocks) {
        //     errs() << "\n" << it.first << " :\n";
        //     for (auto it1:it.second) {
        //         printValue(it1);
        //     }
        // }

    }

    void initThreadDetails(Module &M, 
        map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> &feasibleInterfences
    ) {
        unordered_map<Function*, forward_list<pair<Instruction*, string>>> allLoads;
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores;

        // funcToTCreate: func -> tcreate of func
        map<Function*, Instruction*> funcToTCreate;
        // funcToTJoin: func -> tjoin of func
        map<Function*, Instruction*> funcToTJoin;

        initThreadDetailsHelper(M, allLoads, allStores, funcToTCreate, funcToTJoin);
        // errs() << "\nAll Loads:\n";
        // for(auto it1=allLoads.begin(); it1!=allLoads.end(); ++it1) {
        //     errs() << "Function " << it1->first->getName() << "\n";
        //     auto loadsOfFun = it1->second;
        //     for (auto it2=loadsOfFun.begin(); it2!=loadsOfFun.end(); ++it2) {
        //         printValue(it2->first);
        //     }
        // }

        // errs() << "getting feasible\n";
        getFeasibleInterferences(allLoads, allStores, funcToTCreate, funcToTJoin, feasibleInterfences);
    }

    void initThreadDetails(Module &M, 
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> &feasibleInterfences
    ) {
        unordered_map<Function*, forward_list<pair<Instruction*, string>>> allLoads;
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores;

        // funcToTCreate: func -> tcreate of func
        map<Function*, Instruction*> funcToTCreate;
        // funcToTJoin: func -> tjoin of func
        map<Function*, Instruction*> funcToTJoin;

        initThreadDetailsHelper(M, allLoads, allStores, funcToTCreate, funcToTJoin);
		// errs() << "Init Helper returned\n";
        // errs() << "\nAll Loads:\n";
        // for(auto it1=allLoads.begin(); it1!=allLoads.end(); ++it1) {
        //     errs() << "Function " << it1->first->getName() << "\n";
        //     auto loadsOfFun = it1->second;
        //     for (auto it2=loadsOfFun.begin(); it2!=loadsOfFun.end(); ++it2) {
        //         printValue(it2->first);
        //     }
        // }

        // errs() << "computing feasible interferences\n";
        getFeasibleInterferences(allLoads, allStores, funcToTCreate, funcToTJoin, feasibleInterfences);
		// errs() << "feasible interfs found\n";
    }

    void analyzeProgram(Module &M,
        map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> &feasibleInterfences
    ) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unordered_map <Function*, unordered_map<Instruction*, Environment*>> programStateCurItr;
        bool isFixedPointReached = false;

        map< Function*, vector< map< Instruction*, pair<Instruction*, Environment>>>> oldInterfs; 

        while (!isFixedPointReached) {
            programState.clear();
            programState = programStateCurItr;

            if (!noPrint) {
                errs() << "_________________________________________________\n";
                errs() << "Iteration: " << iterations << "\n";
                // printProgramState();
            }
            // errs() << "Iteration: " << iterations << "\n";
            // #pragma omp parallel for shared(feasibleInterfences,programStateCurItr) private(funcItr) num_threads(threads.size()) chuncksize(1)
            #pragma omp parallel num_threads(threads.size()) if (maxFeasibleInterfs > 200)
            #pragma omp single
            {
            for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr) {
            // for(vector<Function*>::iterator funcItr = std::begin(threads); funcItr != std::end(threads); funcItr++) {
                Function *curFunc = (*funcItr);
                if (!noPrint) {
                    errs() << "\n******** DEBUG: Analyzing thread " << curFunc->getName() << "********\n";
                }
                
                // find feasible interfernce for current function
                vector <forward_list<const pair<Instruction*, Instruction*>*>> curFuncInterfs;
                unordered_map<Instruction*, Environment*> newFuncEnv;
                
                // errs() << "Init Env:\n";
                // searchCurFuncInitEnv->second.printEnvironment();
                
                auto searchInterf = feasibleInterfences.find(curFunc);           
                if (searchInterf != feasibleInterfences.end()) {
                    curFuncInterfs = (searchInterf->second);
                    programStateCurItr[curFunc].clear();
                } else {
                    if (!noPrint) errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        forward_list<const pair<Instruction*, Instruction*>*> interf;
                        newFuncEnv = analyzeThread(curFunc, interf);
                        programStateCurItr.emplace(curFunc, newFuncEnv);
                    }
                    else {
                        auto searchCurFuncInitEnv = funcInitEnv.find(curFunc);
                        assert(searchCurFuncInitEnv!=funcInitEnv.end() && "Intial Environment of the function not found");
                        searchCurFuncInitEnv->second->setNotModified();
                    }
                }
                
                // if (searchCurFuncInitEnv->second.isModified()) { 
                    // init env has changed. analyze for all interfs
                    // errs() << "Number of interf= " << curFuncInterfs.size();
                    // analyze the Thread for each interference
                    // #pragma omp parallel for shared(programStateCurItr) private(newFuncEnv) chunksize(500)
                    #pragma omp task if(curFuncInterfs.size() > 200) \
                    shared(programStateCurItr) firstprivate(curFuncInterfs,funcItr,curFunc) private(newFuncEnv)
                    {
                    for (auto interfItr=curFuncInterfs.begin(); interfItr!=curFuncInterfs.end(); ++interfItr){
                        // errs() << "\n***Forinterf\n";
                        // errs() << "analyzing with interf:\n";
                        // for (auto listItr: (*interfItr)){
                        //     printValue(listItr->first); errs() << " ---> ";
                        //     printValue(listItr->second); errs() << "\n";
                        // }

                        newFuncEnv = analyzeThread(*funcItr, (*interfItr));

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
                // }
                    }
            }
            }
            if (iterations == 0) {
                // errs() << "MOD: setting main() init to false\n";
                funcInitEnv[getMainFunction(M)]->setNotModified();
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
            printProgramState(programState);
        }
    }

    void analyzeProgram(Module &M,
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> &feasibleInterfences
    ) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unordered_map <Function*, unordered_map<Instruction*, Environment*>> programStateCurItr;
        bool isFixedPointReached = false;

        while (!isFixedPointReached ) {
            programState.clear();
			// double itr_start_time = omp_get_wtime();
            programState = programStateCurItr;
            //programStateCurItr.clear();

            if (!noPrint) {
                errs() << "_________________________________________________\n";
                errs() << "Iteration: " << iterations << "\n";
                // printProgramState();
                // errs() << "-------------------------------------------------\n";
            }
            // errs() << "Iteration: " << iterations << "\n"; //**
            // #pragma omp parallel for shared(feasibleInterfences,programStateCurItr) private(funcItr) num_threads(threads.size()) chuncksize(1)
            #pragma omp parallel num_threads(threads.size()) if (maxFeasibleInterfs > 200)
            #pragma omp single
            {
            for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
            // for(vector<Function*>::iterator funcItr = std::begin(threads); funcItr != std::end(threads); funcItr++) {
                Function *curFunc = (*funcItr);
                if (!noPrint) {
                    errs() << "\n******** DEBUG: Analyzing thread " << curFunc->getName() << "********\n";
                }
                
                // find feasible interfernce for current function
                vector<pair<Instruction*, vector<Instruction*>>> curFuncInterfs;
                unordered_map<Instruction*, Environment*> newFuncEnv;
                
                // errs() << "Init Env:\n";
                // searchCurFuncInitEnv->second.printEnvironment();
                
                auto searchInterf = feasibleInterfences.find(curFunc);           
                if (searchInterf == feasibleInterfences.end()) {
                    if (!noPrint) errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        // forward_list<const pair<Instruction*, Instruction*>*> interf;
                        // newFuncEnv = analyzeThread(curFunc, interf);
                        // programStateCurItr.emplace(curFunc, newFuncEnv);
                        // continue;
                    }
                    else {
                        auto searchCurFuncInitEnv = funcInitEnv.find(curFunc);
                        assert(searchCurFuncInitEnv!=funcInitEnv.end() && "Intial Environment of the function not found");
                        searchCurFuncInitEnv->second->setNotModified();
                    }
                }
                else {
                    curFuncInterfs = (searchInterf->second);
                    // programStateCurItr[curFunc].clear();  // will cause problems in parallelization
                    // if (searchCurFuncInitEnv->second.isModified()) { 
                    // init env has changed. analyze for all interfs
                }
                newFuncEnv = analyzeThread(*funcItr, curFuncInterfs);

                // join newFuncEnv of all feasibleInterfs and replace old one in state
                // programStateCurItr.erase(curFunc);
                programStateCurItr[curFunc] = newFuncEnv;
            }
            }
            if (iterations == 0) {
                // errs() << "MOD: setting main() init to false\n";
                funcInitEnv[getMainFunction(M)]->setNotModified();
            }
            
            // errs() << "------ Program state at iteration " << iterations << ": -----\n";
            // for (auto it: programStateCurItr) {
            //     for (auto it2: it.second) {
            //         printValue(it2.first);
			// 		if (it2.second != nullptr) it2.second->printEnvironment();
			// 		else errs() << "NULL\n";
            //     }
            // }

            // isFixedPointReached = true;
            // TODO: fixed point is reached when no new interf for any func found and the init env has not changed
            isFixedPointReached = isFixedPoint(programStateCurItr);
            iterations++;
			// fprintf(stderr, "time: %f\n", omp_get_wtime() - itr_start_time);
        }
        if (!noPrint) {
            errs() << "_________________________________________________\n";
            errs() << "Fixed point reached in " << iterations << " iterations\n";
            errs() << "Final domain:\n";
            // printProgramState();
        }
    }

    unordered_map<Instruction*, Environment*> analyzeThread (Function *F, 
        forward_list<const pair<Instruction*, Instruction*>*> &interf
    ) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, Environment*> curFuncEnv;
        // errs() << "MOD: setting curfuncEnv, duncinitENv modified: " << funcInitEnv[F].isModified() <<"\n";
        curFuncEnv[&(F->getEntryBlock().front())] = funcInitEnv[F];
        auto curInterfItr = interf.begin();
        // errs() << "CurFuncEnv before checking preds:\n";
        // printInstToEnvMap(curFuncEnv);
        queue<BasicBlock*> basicBlockQ;
        unordered_set<BasicBlock*> basicBlockSet;
        basicBlockQ.push(&*F->begin());
        basicBlockSet.insert(&*F->begin());
        unordered_set<BasicBlock*> done;
        // cmp instr will add the corresponding true and false branch environment to branchEnv. 
        // cmpVar -> (true branch env, false branch env)
        map<Instruction*, pair<Environment*, Environment*>> branchEnv;

        while (!basicBlockQ.empty()) {
            BasicBlock* BB = basicBlockQ.front();
            basicBlockQ.pop();

            // check if all pred BBs are analyzed
            bool doAnalyze = true;
            for (auto predBB: predecessors(BB)) {
                auto searchPred = done.find(predBB);
                if (searchPred == done.end()) {
                    doAnalyze = false;
                    break;
                }
            }
            if (doAnalyze) {
                analyzeBasicBlock(BB, curFuncEnv, &curInterfItr, interf.end(), branchEnv);
                done.insert(BB);
            }
            else {
                basicBlockQ.push(BB);
            }

            for (auto succBB: successors(BB)) {
                if (basicBlockSet.insert(succBB).second)
                    basicBlockQ.push(succBB);
            }
        }

        return curFuncEnv;
    }

    unordered_map<Instruction*, Environment*> analyzeThread (Function *F, 
        vector<pair<Instruction*, vector<Instruction*>>> &interf
    ) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, Environment*> curFuncEnv;
        // errs() << "MOD: setting curfuncEnv, duncinitENv modified: " << funcInitEnv[F].isModified() <<"\n";
        curFuncEnv[&(F->getEntryBlock().front())] = funcInitEnv[F];
        auto curInterfItr = interf.begin();
        // errs() << "CurFuncEnv before checking preds:\n";
        // printInstToEnvMap(curFuncEnv);
        queue<BasicBlock*> basicBlockQ;
        unordered_set<BasicBlock*> basicBlockSet;
        basicBlockQ.push(&*F->begin());
        basicBlockSet.insert(&*F->begin());
        unordered_set<BasicBlock*> done;
        // cmp instr will add the corresponding true and false branch environment to branchEnv. 
        // cmpVar -> (true branch env, false branch env)
        map<Instruction*, pair<Environment*, Environment*>> branchEnv;
        
        while (!basicBlockQ.empty()) {
            BasicBlock* BB = basicBlockQ.front();
            basicBlockQ.pop();

            // check if all pred BBs are analyzed
            bool doAnalyze = true;
            for (auto predBB: predecessors(BB)) {
                auto searchPred = done.find(predBB);
                if (searchPred == done.end()) {
                    doAnalyze = false;
                    break;
                }
            }
            if (doAnalyze) {
                analyzeBasicBlock(BB, curFuncEnv, &curInterfItr, interf.end(), branchEnv);
                done.insert(BB);
            }
            else {
                basicBlockQ.push(BB);
            }

            for (auto succBB: successors(BB)) {
                if (basicBlockSet.insert(succBB).second)
                    basicBlockQ.push(succBB);
            }
        }
		// errs () << "Env of Thread " << F->getName() << ":\n";
		// printInstToEnvMap(curFuncEnv);

        return curFuncEnv;
    }

    void analyzeBasicBlock (BasicBlock *B, 
        unordered_map <Instruction*, Environment*> &curFuncEnv,
        forward_list<const pair<Instruction*, Instruction*>*>::iterator *curInterfItr,
        forward_list<const pair<Instruction*, Instruction*>*>::const_iterator endCurInterfItr,
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv
    ) {
        // check type of inst, and performTrasformations
        // Environment curEnv;
        Environment *predEnv = curFuncEnv[&(*(B->begin()))];
        // errs() << "MOD: predEnv modified:" << predEnv.isModified() << "\n";
        
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);
			Environment *curEnv=new Environment();
            curEnv->copyEnvironment(*predEnv);
        
            if (!noPrint) {
                errs() << "DEBUG: Analyzing: ";
                printValue(currentInst);
            }

            // We need to check unaryInst (loads), callInst(thread create, join) and 
            // rmw inst even if current environmet's modified flag is unset becausee
            // these are the instructions that are affected by interferences and the 
            // flag might change if the corresponding flag in the other thread is set.
            if (LoadInst *loadInst = dyn_cast<LoadInst>(instItr)) {
                // errs() << "checking unary inst:"; printValue(loadInst);
                curEnv = checkUnaryInst(loadInst, curEnv, curInterfItr, endCurInterfItr);
            }
            else if(AtomicRMWInst *rmwInst = dyn_cast<AtomicRMWInst>(instItr)) {
                // errs() << "initial env:\n";
                // curEnv.printEnvironment();
                curEnv = checkRMWInst(rmwInst, curEnv, curInterfItr, endCurInterfItr);
            }
            else curEnv = checkNonInterfInsts(currentInst, curEnv, branchEnv, curFuncEnv);

            curFuncEnv.emplace(currentInst, curEnv);
			predEnv = curEnv;
            if (!noPrint) curEnv->printEnvironment();
        }
        
    }

    void analyzeBasicBlock (BasicBlock *B, 
        unordered_map <Instruction*, Environment*> &curFuncEnv,
        vector<pair<llvm::Instruction*, vector<llvm::Instruction*>>>::iterator *curInterfItr,
        vector<pair<llvm::Instruction*, vector<llvm::Instruction*>>>::const_iterator endCurInterfItr,
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv
    ) {
        // check type of inst, and performTrasformations
        // Environment curEnv;
        Environment *predEnv = curFuncEnv[&(*(B->begin()))];
        // errs() << "MOD: predEnv modified:" << predEnv.isModified() << "\n";
        // cmp instr will add the corresponding true and false branch environment to branchEnv. 
        // cmpVar -> (true branch env, false branch env)
        
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);
            Environment *curEnv=new Environment();
            if (!noPrint) {
                errs() << "DEBUG: Analyzing: ";
                printValue(currentInst);
            }
			curEnv->copyEnvironment(*predEnv);
        

            // We need to check unaryInst (loads), callInst(thread create, join) and 
            // rmw inst even if current environmet's modified flag is unset becausee
            // these are the instructions that are affected by interferences and the 
            // flag might change if the corresponding flag in the other thread is set.
            if (LoadInst *loadInst = dyn_cast<LoadInst>(instItr)) {
                // errs() << "checking unary inst:"; printValue(loadInst);
                curEnv = checkUnaryInst(loadInst, curEnv, curInterfItr, endCurInterfItr);
                // if (iterations >= 4) curEnv.printEnvironment();
            }
            else if(AtomicRMWInst *rmwInst = dyn_cast<AtomicRMWInst>(instItr)) {
                // errs() << "initial env:\n";
                // curEnv.printEnvironment();
                curEnv = checkRMWInst(rmwInst, curEnv, curInterfItr, endCurInterfItr);
                // if (iterations >= 4) curEnv.printEnvironment();
            }
        	else if (isLockInst(&(*instItr))) {
				// errs() << "Lock: "; printValue(callInst);
				// errs() << "Before call:\n"; curEnv.printEnvironment();
				if (llvm::CallInst *call = llvm::dyn_cast<llvm::CallInst>(instItr)) {
                	curEnv = checkAcquireLock(call, curEnv);
				}
				// checkAcquireLock(callInst, curEnv);
				// errs() << "After call:\n"; curEnv.printEnvironment();
				// return curEnv;
			}
            else curEnv = checkNonInterfInsts(currentInst, curEnv, branchEnv, curFuncEnv);

			// errs() << "After call:\n"; curEnv.printEnvironment();
			// errs() << "Assigning to map\n";
            curFuncEnv[currentInst]= curEnv;
			// errs() << "Assigned\n";
			predEnv = curEnv;
			if (!noPrint) {predEnv->printEnvironment();}
            // if (!noPrint) errs() << "size: " << predEnv.size() << "\n";
			// printInstToEnvMap(curFuncEnv);
        }
		// errs() << "Env of BB:\n"; printValue(B);
		// printInstToEnvMap(curFuncEnv);
        
        // return curEnv;
    }

	// TODO: no need to return env
    Environment* checkNonInterfInsts (Instruction *inst, Environment *curEnv,
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv,
        unordered_map <Instruction*, Environment*> &curFuncEnv
    ) {
		// errs() << "Non interf inst\n";
        if (CallInst *callInst = dyn_cast<CallInst>(inst)) {
            if (stopOnFail && callInst->getCalledFunction()->getName() == "__assert_fail") {
                // errs() << "*** found assert" << "\n";
                // printValue(callInst);
                if (!curEnv->isUnreachable()) {
                    errs() << "Assertion failed:\n";
                    printValue(callInst);
                    if (!noPrint) {
                        curEnv->printEnvironment();
                    }
                    errs() << "___________________________________________________\n";
                    errs() << "# Failed Asserts: 1\n";
                    double time = omp_get_wtime() - start_time;
                    fprintf(stderr, "Time elapsed: %f\n", time);
                    fprintf(stderr, "#iterations: %d\n", iterations+1);
			// compareProgramState(mergeProgramState, programState);
                    exit(0);
                }
            }
            else if(!callInst->getCalledFunction()->getName().compare("pthread_create")) {
                if (Function* newThread = dyn_cast<Function> (callInst->getArgOperand(2))) {
                    // Change the funcInitEnv of newThread
                    checkThreadCreate(callInst, curEnv);
                    return curEnv;
                }
            }
            else if (!callInst->getCalledFunction()->getName().compare("pthread_join")) {
                // errs() << "Env before thread join:\n";
                // curEnv.printEnvironment();
                return checkThreadJoin(callInst, curEnv);
                // errs() << "Env after thread join:\n";
                // curEnv.printEnvironment();
            }
            else if (callInst->getCalledFunction()->getName() == "_Z6assumeb") {
                // errs() << "Assume function call: "; printValue(callInst);
                // errs() << "Env before Assume call:\n";
                // curEnv.printEnvironment();
                return checkAssumeCall(callInst, curEnv, branchEnv);
                // errs() << "Env after Assume call:\n";
                // curEnv.printEnvironment();

            }
			else if (callInst->getCalledFunction()->getName() == "_Z10nondet_intRSt6atomicIiEii" ||
					 callInst->getCalledFunction()->getName() == "_Z10nondet_intRiii") {
				// errs() << "nondet_int found\n";
				return checkNonDetIntCall(callInst, curEnv);
			}
            else if (callInst->getCalledFunction()->getName().find("unlock")!=llvm::StringRef::npos) {
                // errs() << "Lock: "; printValue(callInst);
				// errs() << "Before call:\n"; curEnv.printEnvironment();
				return checkReleaseLock(callInst, curEnv);
                // checkReleaseLock(callInst, curEnv);
				// errs() << "After call:\n";
				//curEnv.printEnvironment();
				// return curEnv;
            }
        }
        // Other instructions don't need to be re-checked if modified flag is unset
        else if (PHINode *phinode = dyn_cast<PHINode>(inst)) {
            // errs() << "all binop in branchenv:\n";
            // for (auto it=branchEnv.begin(); it!=branchEnv.end(); it++) {
            //     printValue(it->first);
            //     if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(it->first)) {
            //         auto oper = binOp->getOpcode();
            //         if (oper == Instruction::And || oper == Instruction::Or) {
            //             errs() << "bin Op. ";
            //             errs() << "true:\n";
            //             it->second.first.printEnvironment();
            //             errs() << "false:\n";
            //             it->second.second.printEnvironment();
            //         }
            //     }
            // }
            // errs() << "Debug: Analyzing phi node: "; printValue(phinode); 
            return checkPhiNode(phinode, curEnv, branchEnv, curFuncEnv);
        }
        // else if (!curEnv->isModified()) {
        //     curEnv = programState[inst->getFunction()][inst];
        //     // errs() << "MOD: not analyzing further. Returning\n";
        //     // programState[inst->getFunction()][inst].printEnvironment();
        //     return curEnv;
        //     // if(curEnv.isModified()) curEnv.setNotModified();
        // }
        // All the instructions need to be re-analyzed if modification flag is set.
        else if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(inst)) {
            // auto searchName = valueToName.find(currentInst);
            // if (searchName == valueToName.end()) {
            //     fprintf(stderr, "ERROR: new mem alloca");
            //     allocaInst->dump();
            //     exit(0);
            // }
        }
        else if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(inst)) {
            auto oper = binOp->getOpcode();
            if (oper == Instruction::And || oper == Instruction::Or) {
                Environment* env = checkLogicalInstruction(binOp, curEnv, branchEnv);
                // errs() << "True branch Env:\n";
                // branchEnv[inst].first.printEnvironment();
                // errs() << "False branch Env:\n";
                // branchEnv[inst].second.printEnvironment();
                return env;
            }
            else return checkBinInstruction(binOp, curEnv);
        }
        else if (StoreInst *storeInst = dyn_cast<StoreInst>(inst)) {
            return checkStoreInst(storeInst, curEnv);
        }
		else if (SelectInst *selInst = dyn_cast<SelectInst>(inst)) {
			return checkSelectInst(selInst, curEnv, branchEnv);
		}
        else if (CmpInst *cmpInst = dyn_cast<CmpInst> (inst)) {
            // errs() << "Env before cmp:\n";
            // curEnv.printEnvironment();
            Environment* env =  checkCmpInst(cmpInst, curEnv, branchEnv);
            // errs() << "\nCmp result:\n";
            // curEnv.printEnvironment();
            // errs() <<"True Branch:\n";
            // branchEnv[cmpInst].first->printEnvironment();
            // errs() <<"False Branch:\n";
            // branchEnv[cmpInst].second->printEnvironment();
			// errs() <<"Done\n";
            return env;
        }
        else if (BranchInst *branchInst = dyn_cast<BranchInst>(inst)) {
            if (branchInst->isConditional()) {
                Instruction *branchCondition = dyn_cast<Instruction>(branchInst->getCondition());
                Instruction *trueBranch = &(*(branchInst->getSuccessor(0)->begin()));
				// search env of true branch in currently explred program state of this iteration
				auto searchBranchEnv = branchEnv.find(branchCondition);
				if (searchBranchEnv == branchEnv.end()) {
					errs() << "ERROR: Branch Condition not found\n";
					exit(0);
				}
				auto searchTrueEnv = curFuncEnv.find(trueBranch);
				if (searchTrueEnv == curFuncEnv.end()) {
					// assign from the branch env
					Environment *tmpEnv = new Environment();
					tmpEnv->copyEnvironment(*searchBranchEnv->second.first);
					curFuncEnv.emplace(make_pair(trueBranch, tmpEnv));
				}
				else {
					// if it already exit, merge
					searchTrueEnv->second->joinEnvironment(*searchBranchEnv->second.first);
				}
                Instruction *falseBranch = &(*(branchInst->getSuccessor(1)->begin()));
				// search env of false branch in currently explred program state of this iteration
				auto searchFalseEnv = curFuncEnv.find(falseBranch);
				if (searchFalseEnv == curFuncEnv.end()) {
					// assign from the branch env
					Environment *tmpEnv = new Environment();
					tmpEnv->copyEnvironment(*searchBranchEnv->second.second);
					curFuncEnv.emplace(make_pair(falseBranch, tmpEnv));
				}
				else {
					// if it already exit, merge
					searchFalseEnv->second->joinEnvironment(*searchBranchEnv->second.second);
				}
                // errs() << "\nTrue Branch:\n";
                // printValue(trueBranch);
                // errs() << "True branch Env:\n";
                // curFuncEnv[trueBranch]->printEnvironment();
                // errs() << "\nFalse Branch:\n";
                // printValue(falseBranch);
                // errs() << "False branch Env:\n";
                // curFuncEnv[falseBranch]->printEnvironment();
				// errs() << "Done\n";
            }
            else {
                Instruction *successors = &(*(branchInst->getSuccessor(0)->begin()));
				// search succ env in curFuncEnv
				auto searchSuccEnv = curFuncEnv.find(successors);
				if (searchSuccEnv == curFuncEnv.end()) {
					curFuncEnv.emplace(make_pair(successors, curEnv));
				}
				else {
                	curFuncEnv[successors]->joinEnvironment(*curEnv);
				}
            }
        } 
		else if (SwitchInst *switchInst = dyn_cast<SwitchInst>(inst)) {
			// errs() << "SwitchInst: "; printValue(switchInst);
			// errs() << "#cases: " << switchInst->getNumCases() << "\n";
			// errs() << "Codition: "; printValue(switchInst->getCondition());
			string condVar = getNameFromValue(switchInst->getCondition());
			// errs() << "Condition Var: " << condVar << "\n";
			// Environment tmpEnv;
			// tmpEnv.copyEnvironment(*curEnv);
			// errs() << "curEnv before switch:\n";
			// curEnv.printEnvironment();
			Environment *defaultEnv = new Environment();
			for (auto caseItr=switchInst->case_begin(); caseItr!=switchInst->case_end(); caseItr++) {
				// errs() << "Case " << caseItr->getCaseIndex() << ": "; 
				// printValue(caseItr->getCaseValue());
				// printValue(caseItr->getCaseSuccessor());
            	int compConst= caseItr->getCaseValue()->getSExtValue();
				Environment *caseEnv = new Environment();
				caseEnv->copyEnvironment(*curEnv);
				caseEnv->performCmpOp(operation::EQ, condVar, compConst);
				Environment tmpEnv; 
				tmpEnv.copyEnvironment(*curEnv);
				tmpEnv.performCmpOp(operation::NE, condVar, compConst);
				defaultEnv->joinEnvironment(tmpEnv);
				// errs() << "curEnv after cmp:\n"; 
				// curEnv.printEnvironment();
				Instruction *caseSuccessor = &(*(caseItr->getCaseSuccessor()->begin()));
				// errs() << "Successor env before join: \n";
				// curFuncEnv[caseSuccessor].printEnvironment();
				// search case succ env
				auto searchCaseEnv = curFuncEnv.find(caseSuccessor);
				if (searchCaseEnv == curFuncEnv.end()) {
					// assign the caseEnv
					curFuncEnv.emplace(make_pair(caseSuccessor, caseEnv));
				}
				else {
					// if it already exist, merge
					curFuncEnv[caseSuccessor]->joinEnvironment(*caseEnv);
					delete caseEnv;			
				}
				// errs() << "Successor env after case:\n";
				// curFuncEnv[caseSuccessor]->printEnvironment();
			}
			// set environment in default destination of switch-case
			Instruction *defaultDest = &(*(switchInst->getDefaultDest()->begin()));
			auto searchDefaultCaseEnv = curFuncEnv.find(defaultDest);
			if (searchDefaultCaseEnv == curFuncEnv.end()) {
				// assign the defaultEnv to defaultCase
				curFuncEnv.emplace(make_pair(defaultDest, defaultEnv));
			}
			else {
				// it already exist, merge
				curFuncEnv[defaultDest]->joinEnvironment(*defaultEnv);
				delete defaultEnv;
			}
			curFuncEnv[defaultDest] = defaultEnv;
		}
        // CMPXCHG
        else {
            
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
            errs() << "ERROR: Instrution not found in Instruction to Name map: ";
            printValue(val);
            // exit(0);
            return "";
        }
        return searchName->second;
    }

    Environment* checkCmpInst (
        CmpInst* cmpInst, 
        Environment *curEnv, 
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv
    ) { 
        // need to computer Environment of both true and false branch
        Environment *trueBranchEnv=new Environment();
        Environment *falseBranchEnv=new Environment();
        trueBranchEnv->copyEnvironment(*curEnv);
        falseBranchEnv->copyEnvironment(*curEnv);

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
			case CmpInst::Predicate::ICMP_UGT:
				operTrueBranch = UGT;
				operFalseBranch = ULE;
				break;
			case CmpInst::Predicate::ICMP_UGE:
				operTrueBranch = UGE;
				operFalseBranch = ULT;
				break;
			case CmpInst::Predicate::ICMP_ULT:
				operTrueBranch = ULT;
				operFalseBranch = UGE;
				break;
			case CmpInst::Predicate::ICMP_ULE:
				operTrueBranch = ULE;
				operFalseBranch = UGT;
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
                trueBranchEnv->performCmpOp(operTrueBranch, constFromIntVar1, constFromIntVar2);
                falseBranchEnv->performCmpOp(operFalseBranch, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                trueBranchEnv->performCmpOp(operTrueBranch, constFromIntVar1, fromVar2Name);
                falseBranchEnv->performCmpOp(operFalseBranch, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            trueBranchEnv->performCmpOp(operTrueBranch, fromVar1Name, constFromIntVar2);
            falseBranchEnv->performCmpOp(operFalseBranch, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
			// errs() << "true branch:\n";
            trueBranchEnv->performCmpOp(operTrueBranch, fromVar1Name, fromVar2Name);
			// errs() << "false branch:\n";
            falseBranchEnv->performCmpOp(operFalseBranch, fromVar1Name, fromVar2Name);
        }
		
		// errs() << "True Branch in cmpInst:\n"; trueBranchEnv.printEnvironment();
		// errs() << "False Branch in cmpInst:\n"; falseBranchEnv.printEnvironment();

        // set the value of destination variable in Environment
        if (trueBranchEnv->isUnreachable()) {
            curEnv->unsetVar(destVarName);
        } else if (falseBranchEnv->isUnreachable()) {
            curEnv->setVar(destVarName);
        }
        trueBranchEnv->setVar(destVarName);
        falseBranchEnv->unsetVar(destVarName);
        
        branchEnv.emplace(make_pair(cmpInst, make_pair(trueBranchEnv, falseBranchEnv)));
        return curEnv;
    }

    Environment* checkLogicalInstruction (
        BinaryOperator* logicalOp, 
        Environment *curEnv, 
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv
    ) {
        // errs() << "Logical Op. Size of curEnv: " << curEnv.size() << "\n";
		string destVarName = getNameFromValue(logicalOp);
        Value* fromVar1 = logicalOp->getOperand(0);
        Value* fromVar2 = logicalOp->getOperand(1);
        
        Environment fromVar1TrueEnv;
        Environment fromVar1FalseEnv;
        Environment fromVar2TrueEnv;
        Environment fromVar2FalseEnv;
        
        if (CmpInst *op1 = dyn_cast<CmpInst>(fromVar1)) {
			auto searchBranchEnv = branchEnv.find(op1);
			if (searchBranchEnv == branchEnv.end()) {
				errs() << "ERROR: op1 not found\n";
				exit(0);
			}
            fromVar1TrueEnv = *searchBranchEnv->second.first;
            fromVar1FalseEnv = *searchBranchEnv->second.second;
        }
        else if (BinaryOperator *op1 = dyn_cast<BinaryOperator>(fromVar1)) {
            auto oper = op1->getOpcode();
            if (oper == Instruction::And || oper == Instruction::Or) {
				auto searchBranchEnv = branchEnv.find(op1);
				if (searchBranchEnv == branchEnv.end()) {
					errs() << "ERROR: op1 not found\n";
					exit(0);
				}
                fromVar1TrueEnv = *searchBranchEnv->second.first;
                fromVar1FalseEnv = *searchBranchEnv->second.second;
            }
        }
        else {
            errs() << "ERROR: env not found in branchEnv\n";
            printValue(fromVar1);
            exit(0);
        }
        if (CmpInst *op2 = dyn_cast<CmpInst>(fromVar2)) {
			auto searchBranchEnv = branchEnv.find(op2);
			if (searchBranchEnv == branchEnv.end()) {
				errs() << "ERROR: op2 not found\n";
				exit(0);
			}
            fromVar2TrueEnv = *searchBranchEnv->second.first;
            fromVar2FalseEnv = *searchBranchEnv->second.second;
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
				auto searchBranchEnv = branchEnv.find(op2);
				if (searchBranchEnv == branchEnv.end()) {
					errs() << "ERROR: op2 not found\n";
					exit(0);
				}
                fromVar2TrueEnv = *searchBranchEnv->second.first;
                fromVar2FalseEnv = *searchBranchEnv->second.second;
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
        Environment *trueBranchEnv=new Environment();
        Environment *falseBranchEnv=new Environment();
        trueBranchEnv->copyEnvironment(fromVar1TrueEnv);
        falseBranchEnv->copyEnvironment(fromVar1FalseEnv);
        if (oper == Instruction::And) {
            // errs() << "taking meet of - trueBranch\n";
			// trueBranchEnv.printEnvironment(); fromVar2TrueEnv.printEnvironment();
			// errs() << "fromVar2True: \n";
            trueBranchEnv->meetEnvironment(fromVar2TrueEnv);
            // errs() << "taking join\n";
            falseBranchEnv->joinEnvironment(fromVar2FalseEnv);
			// errs() << "join done\n";
        }

        else if (oper == Instruction::Or) {
            //-
            // errs() << "T1:\n";
            // trueBranchEnv.printEnvironment();
            // errs() << "T2:\n";
            // fromVar2TrueEnv.printEnvironment();
            // -//
            trueBranchEnv->joinEnvironment(fromVar2TrueEnv);
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
            falseBranchEnv->meetEnvironment(fromVar2FalseEnv);
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
        if (trueBranchEnv->isUnreachable()) {
            curEnv->unsetVar(destVarName);
        } else if (falseBranchEnv->isUnreachable()){
            curEnv->setVar(destVarName);
        }
        trueBranchEnv->setVar(destVarName);
        falseBranchEnv->unsetVar(destVarName);

        branchEnv.emplace(make_pair(logicalOp, make_pair(trueBranchEnv, falseBranchEnv)));
		// curEnv.printEnvironment();
        return curEnv;
    }


    Environment* checkPhiNode (
        PHINode* phinode,
        Environment *curEnv,
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv,
		const unordered_map<Instruction*, Environment*> &curFuncEnv 
    ) {
        unsigned int numIncoming = phinode->getNumIncomingValues(); 
        // errs() << "Phi Node. #incoming edges: " << numIncoming << "\n";
        string destVarName = getNameFromValue(phinode);

        for (auto i=0; i<numIncoming; i++) {
            // errs() << "Terminator of "<< i << "th BB: ";
            // printValue(phinode->getIncomingBlock(i)->getTerminator());
            Environment phiEnv;
            TerminatorInst *termInst = phinode->getIncomingBlock(i)->getTerminator();
            if (BranchInst *branch = dyn_cast<BranchInst>(termInst)) {
                if (branch->isConditional()) {
					// errs() << "conditional branch\n";
                    Instruction *branchCondition = dyn_cast<Instruction>(branch->getCondition());
                    if (branch->getSuccessor(0) == phinode->getParent()) {
						// errs() << "copying from true branchEnv\n";
                        phiEnv.copyEnvironment(*branchEnv[branchCondition].first);
                    }
                    else {
                        assert(branch->getSuccessor(1) == phinode->getParent() && "We do not support branch with more than two successors");
						// errs() << "copying from false branchEnv\n";
						// errs() << "phiEnv before copying:\n"; phiEnv.printEnvironment();
						// errs() << "Copying from:\n"; branchEnv[branchCondition].second->printEnvironment();
                        phiEnv.copyEnvironment(*branchEnv[branchCondition].second);
                    }
                }
                else {
                    phiEnv.copyEnvironment(*curFuncEnv.at(branch));
                }
            }
            else {
                phiEnv.copyEnvironment(*curFuncEnv.at(termInst));
            }
            // errs() << "Value of ith BB: "; printValue(phinode->getIncomingValue(i));
            // errs() << "Dest Var: " << destVarName << "\n";
            auto *phiVal = phinode->getIncomingValue(i);

            if (ConstantInt *constInt = dyn_cast<ConstantInt>(phiVal)) {
                if (!phiEnv.isUnreachable()) {
                    // errs() << "Const int width: " << constInt->getBitWidth() << "\n";
                    if (constInt->getBitWidth() == 1) {
                        // errs() << "Bool value: " << constInt->isOne() << "\n";
                        phiEnv.performUnaryOp(STORE, destVarName, constInt->isOne());
                    }
                    else {
                        // errs() << "Const int Value: " << constInt->getSExtValue() << "\n";
                        phiEnv.performUnaryOp(STORE, destVarName, constInt->getSExtValue());
                    }
                }
            }
            else {
                if (CmpInst *cmpInst = dyn_cast<CmpInst>(phiVal)) {
                    phiEnv.copyEnvironment(*branchEnv[cmpInst].first);
                    phiEnv.joinEnvironment(*branchEnv[cmpInst].second);
                }
                else if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(phiVal)) {
                    if (binOp->getOpcode() == Instruction::And || binOp->getOpcode() == Instruction::Or) {
                        // errs() << "found logical inst\n";
                        phiEnv.copyEnvironment(*branchEnv[binOp].first);
                        // errs() << "true env:\n";
                        // phiEnv.printEnvironment();
                        // branchEnv[binOp].first->printEnvironment();
                        // errs() << "false env:\n";
                        // branchEnv[binOp].second->printEnvironment();
                        phiEnv.joinEnvironment(*branchEnv[binOp].second);
                        // errs() << "true+false env:\n";
                        // phiEnv.printEnvironment();
                    }
                }
                string sourceVarName = getNameFromValue(phiVal);
                // errs() << "Source Var: " << sourceVarName << "\n";
                if (!phiEnv.isUnreachable())
                    phiEnv.performUnaryOp(STORE, destVarName, sourceVarName);
                // errs() << "PhiEnv:\n";
                // phiEnv.printEnvironment();
                // errs() << "PhiEnv done\n";
            }
            if (i==0) curEnv->copyEnvironment(phiEnv);
            else {
                // errs() << "Joining: "; curEnv->printEnvironment(); 
                // errs() << "with: "; phiEnv.printEnvironment();
                curEnv->joinEnvironment(phiEnv);
				// errs() << "Done\n";
            }
            // curEnv->printEnvironment();
        }
        return curEnv;
    }

    //  call approprproate function for the inst passed
    Environment* checkBinInstruction(BinaryOperator* binOp, Environment *curEnv) {
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
                curEnv->performBinaryOp(oper, destVarName, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                curEnv->performBinaryOp(oper, destVarName, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            curEnv->performBinaryOp(oper, destVarName, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            curEnv->performBinaryOp(oper, destVarName, fromVar1Name, fromVar2Name);
        }

        return curEnv;
    }

	Environment* checkSelectInst(SelectInst *selInst, Environment *curEnv, 
		map<Instruction*, pair<Environment*, Environment*>> &branchEnv
	) {
		Environment trueEnv, falseEnv;
		string destVarName = getNameFromValue(selInst);
		bool condEvaluated = false;
		if (isa<BinaryOperator>(selInst->getCondition()) ||
				isa<CmpInst>(selInst->getCondition())) {
			// errs() << "BinOp or CmpInst. Getting from branch\n";
			Instruction *condInst = dyn_cast<Instruction>(selInst->getCondition());
			auto searchBranchEnv = branchEnv.find(condInst);
			if (searchBranchEnv == branchEnv.end()) {
				errs() << "Instructions not found in branchEnv: ";
				printValue(selInst->getCondition());
				exit(0);
			}
			trueEnv.copyEnvironment(*searchBranchEnv->second.first);
			falseEnv.copyEnvironment(*searchBranchEnv->second.second);
			condEvaluated = true;
		}
		else {
			// errs() << "Not BinOp or CmpInst. Copying from curEvv\n";
			trueEnv.copyEnvironment(*curEnv);
			falseEnv.copyEnvironment(*curEnv);
		}
		int zero = 0;
		string cond = getNameFromValue(selInst->getCondition());
		// if select condition is false, destVar is set to false value
		if (!condEvaluated) falseEnv.performCmpOp(operation::EQ, cond, zero);
		// errs() << "Select Inst FalseEnv:\n";
		// falseEnv.printEnvironment();
		if (ConstantInt *constFalseVal = dyn_cast<ConstantInt>(selInst->getFalseValue())) {
			// if the type of false value is integer
			// errs() << "False value is int. assigning\n";
			int constFalseIntVal = constFalseVal->getValue().getSExtValue();
			falseEnv.performUnaryOp(STORE, destVarName, constFalseIntVal);
			// errs() << "FalseEnv after assignment:\n";
			// falseEnv.printEnvironment();
		}
		else {
			// if the type is not int
			falseEnv.performUnaryOp(STORE, destVarName, getNameFromValue(selInst->getFalseValue()));
		}

		// if select condition is true, destVar is set to true value
		if (!condEvaluated) trueEnv.performCmpOp(operation::NE, cond, zero);
		// errs() << "Select Inst TrueEnv:\n";
		// trueEnv.printEnvironment();
		if (ConstantInt *constTrueVal = dyn_cast<ConstantInt>(selInst->getTrueValue())) {
			// if the type of true value is integer
			// errs() << "True value is int. assigning\n";
			int constTrueIntVal = constTrueVal->getValue().getSExtValue();
			trueEnv.performUnaryOp(STORE, destVarName, constTrueIntVal);
			// errs() << "TrueEnv after assignment:\n";
			// trueEnv.printEnvironment();
		}
		else {
			// if the type is not int
			trueEnv.performUnaryOp(STORE, destVarName, getNameFromValue(selInst->getTrueValue()));
		}
		// the curEnv should be join of true and false Env
		curEnv->copyEnvironment(falseEnv);
		// errs() << "CurEnv after copying from false:\n";
		// curEnv->printEnvironment();
		curEnv->joinEnvironment(trueEnv);
		// errs() << "CurEnv after joining true:\n";
		// curEnv->printEnvironment();
		return curEnv;
	}

    Environment* checkStoreInst(StoreInst* storeInst, Environment *curEnv) {
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
		// errs() << "destVarName: " << destVarName << "\n";

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
			// check if store is of some global variable
			if (std::find(globalVars.begin(), globalVars.end(), destVarName) != globalVars.end()) {
				// errs() << "destVar is atomic. appending the storeInst in PO\n";
        		curEnv->performStoreOp(getInstNumByInst(storeInst), destVarName);
			}
        // }
        
        #ifdef NOTRA
        }
        else {
            curEnv->changeRelHeadToNull(destVarName, storeInst);
        }
        #endif

        Value* fromVar = storeInst->getValueOperand();
        
        if (ConstantInt *constFromVar = dyn_cast<ConstantInt>(fromVar)) {
			// errs() << "storing const: ";
            int constFromIntVar = constFromVar->getValue().getSExtValue();
			// errs() << constFromIntVar << "\n";
            curEnv->performUnaryOp(STORE, destVarName, constFromIntVar);
        }
        else if (Argument *argFromVar = dyn_cast<Argument>(fromVar)) {
            // TODO: handle function arguments

        }
        else {
            string fromVarName = getNameFromValue(fromVar);
            curEnv->performUnaryOp(STORE, destVarName, fromVarName);
        }
        // curEnv.printEnvironment();
        return curEnv;
    }

    Environment* checkUnaryInst(
        UnaryInstruction* unaryInst, 
        Environment *curEnv, 
        vector<pair<Instruction*, vector<Instruction*>>>::iterator *curInterfItr,
        vector<pair<Instruction*, vector<Instruction*>>>::const_iterator &endCurInterfItr
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
                    curEnv = applyInterfToLoad(unaryInst, curEnv, curInterfItr, endCurInterfItr, fromVarName);
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
        
        curEnv->performUnaryOp(LOAD, destVarName.c_str(), fromVarName.c_str());

        return curEnv;
    }

    Environment* checkUnaryInst(
        UnaryInstruction* unaryInst, 
        Environment *curEnv, 
        forward_list<const pair<Instruction*, Instruction*>*>::iterator *curInterfItr,
        forward_list<const pair<Instruction*, Instruction*>*>::const_iterator &endCurInterfItr
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
                    curEnv = applyInterfToLoad(unaryInst, curEnv, curInterfItr, endCurInterfItr, fromVarName);
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
        
        curEnv->performUnaryOp(LOAD, destVarName.c_str(), fromVarName.c_str());

        return curEnv;
    }

    void checkThreadCreate(CallInst *callInst, Environment *curEnv) {
        // TODO: need to call this oper only if curEnv has changed and set changed to false
        // Carry the environment of current thread to the newly created thread. 
        // Need to copy only globals, discard locals.
        
        if (Function* calledFunc = dyn_cast<Function> (callInst->getArgOperand(2))) {
            if (curEnv->isModified()) {
                funcInitEnv[calledFunc]->copyOnVars(*curEnv, globalVars);
                funcInitEnv[calledFunc]->setModified();
            }
            else {
                funcInitEnv[calledFunc]->setNotModified();
                // errs() << "MOD: Set to false\n";
            }
        }
    }

    Environment* checkThreadJoin(CallInst *callInst, Environment *curEnv) {
        // Join the environments of joinee thread with this thread. 
        // Need to copy only globals, discard locals.
        bool modified = curEnv->isModified();
		bool isRetReachable = false;
        Function *calledFunc = findFunctionFromPthreadJoin(callInst);
		
        // Environment *olderEnv = programState[callInst->getFunction()][callInst];
		// search the env of callInst in programState of last itr
		auto searchFuncOlderEnv = programState.find(callInst->getFunction());
		Environment *olderEnv = nullptr;
		if (searchFuncOlderEnv != programState.end()) {
			auto searchOlderEnv = searchFuncOlderEnv->second.find(callInst);
			if (searchOlderEnv != searchFuncOlderEnv->second.end()) {
				olderEnv = searchOlderEnv->second;
			}
		}
		auto searchFuncEnv = programState.find(calledFunc);
		if (searchFuncEnv == programState.end()) {
			// errs() << "Function not found in last iter Env. Nothing to join\n";
			delete curEnv;
			curEnv = new Environment();
			return curEnv;
		}
        if (modified || olderEnv==nullptr || olderEnv->isUnreachable()) {
			// search env of calledFunc in programState of last itr
            for (auto bbItr=calledFunc->begin(); bbItr!=calledFunc->end(); ++bbItr) {
                for (auto instItr=bbItr->begin(); instItr!=bbItr->end(); ++instItr) {
                    if (ReturnInst *retInst = dyn_cast<ReturnInst>(instItr)) {
                        // errs() << "pthread join with (mod): ";
                        // printValue(retInst);
						// search env of retInst in programState of last itr
						auto searchRetEnv = searchFuncEnv->second.find(retInst);
						if (searchRetEnv == searchFuncEnv->second.end()) {
							// errs() << "Ret inst env not found in last itr env. Nothing to join\n";
							break;
						}
						// searchRetEnv->second->printEnvironment();
						if (!searchRetEnv->second->isUnreachable()) {
							// errs() << "Before join:\n"; curEnv->printEnvironment();
							// errs() << "Joining with:\n"; searchRetEnv->second->printEnvironment();
                        	curEnv->joinOnVars(*searchRetEnv->second, globalVars);
							// errs() << "Mod: setting isRetReach to true\n";
							isRetReachable = true;
							// errs() << "After join:\n"; curEnv->printEnvironment();
						}
						// a BB can have at most one return instructon. 
						// So we can exit BB loop
						break;
                    }
                }
            }
        }
        else {
            // errs() << "MOD: joining with selective\n";
            for (auto bbItr=calledFunc->begin(); bbItr!=calledFunc->end(); ++bbItr) {
                for (auto instItr=bbItr->begin(); instItr!=bbItr->end(); ++instItr) {
                    if (ReturnInst *retInst = dyn_cast<ReturnInst>(instItr)) {
                        // errs() << "pthread join with (not mod): ";
                        // printValue(retInst);
                        // programState[calledFunc][retInst]->printEnvironment();
						auto searchRetEnv = searchFuncEnv->second.find(retInst);
						if (searchRetEnv == searchFuncEnv->second.end()) {
							// errs() << "Ret inst env not found in last itr env. Nothing to join\n";
							break;
						}
						// searchRetEnv->second->printEnvironment();
                        if (!searchRetEnv->second->isUnreachable()) {
							if (searchRetEnv->second->isModified()) {
                            	// errs() << "OlderEnv:\n"; olderEnv->printEnvironment();
                            	olderEnv->joinOnVars(*searchRetEnv->second, globalVars);
                            	// errs() << "After join:\n";
                            	// olderEnv->printEnvironment();
                        	}
							// errs() << "not Mod: setting isRetReach to true\n";
							isRetReachable = true;
						}
						// a BB can have at most one return instructon. 
						// So we can exit BB loop
						break;
                    }
                }
            }
            curEnv->copyEnvironment(*olderEnv);
        }
		// if no ret inst is reachable in joined thread
		// the pthread join will wait forever
		// set the curEnv to unreachable
		if (!isRetReachable) {
			// errs() << "No Reachable return instruction found\n";
			delete curEnv;
			curEnv = new Environment();
		}
		// TODO: check if we need to change the modified flag
        return curEnv;
    }

    Environment* checkAcquireLock(CallInst *callInst, Environment *curEnv) {
		auto lockInst = callInst->getArgOperand(0);
		if (BitCastOperator *bcast=dyn_cast<BitCastOperator>(lockInst)) {
			// errs() << "bitcast found\n"; //<< bcast->getNumOperand();
            string lockName = getNameFromValue(bcast->getOperand(0));
            // errs() << "Checking acquire lock of variable " << lockName << "\n";
			// curEnv.printEnvironment();
            // apply interf from all unlock inst of this lock variable
            Environment tmpEnv;
			tmpEnv.copyEnvironment(*curEnv);
            for (auto it=lockVarToUnlocks[lockName].begin(); it!=lockVarToUnlocks[lockName].end(); it++) {
				if ((*it)->getFunction() == callInst->getFunction()) continue;
            	if (!noPrint) {
					errs() << "interf from unlock: ";
            		printValue(*it);
				}
            	auto searchInterfFunc = programState.find((*it)->getFunction());
            	if (searchInterfFunc != programState.end()) {
            	    // errs() << "Interf env found\n";
            	    auto searchInterfEnv = searchInterfFunc->second.find(*it);
            	    if (searchInterfEnv != searchInterfFunc->second.end()) {
            	        // apply the interference
            	        // errs() << "Before Interf lock:\n";
            	        // curEnv.printEnvironment();

            	        if (searchInterfEnv->second->isModified() || curEnv->isModified()) {
            	            // errs () << "Modified. Applying interf from:\n";
							// searchInterfEnv->second.printEnvironment();
            	            tmpEnv.applyInterference(lockName, *searchInterfEnv->second, 
            	                getInstNumByInst(callInst), getInstNumByInst(*it));
							// errs() << "After applying interf:\n";
							// tmpEnv.printEnvironment();
            	        }
            	        else {
            	            // errs() << "MOD: not applying interf\n";
            	            tmpEnv.setNotModified();
            	        }
            	        curEnv->joinEnvironment(tmpEnv);
            	    }
            	}
            }
            // errs() << "After applying interf: \n";
            // curEnv.printEnvironment();

            curEnv->performAcquireLock(lockName, getInstNumByInst(callInst));
			// errs() << "After applying lock:\n";
			// curEnv.printEnvironment();
		}
		// errs() << "Before return in acq lock fun:\n";
		// curEnv.printEnvironment();
		return curEnv;
    }

    Environment* checkReleaseLock(CallInst *callInst, Environment *curEnv) {
		auto lockInst = callInst->getArgOperand(0);
		if (BitCastOperator *bcast=dyn_cast<BitCastOperator>(lockInst)) {
			// errs() << "bitcast found\n"; //<< bcast->getNumOperand();
            string lockName = getNameFromValue(bcast->getOperand(0));
            // errs() << "Checking release lock of variable " << lockName << "\n";
			// curEnv.printEnvironment();
    		curEnv->performReleaseLock(lockName, getInstNumByInst(callInst));
			// curEnv.printEnvironment();
		}
		return curEnv;
    }

    Environment* checkAssumeCall(CallInst *callInst, Environment *curEnv,
        map<Instruction*, pair<Environment*, Environment*>> &branchEnv
    ) {
        if (Instruction* assumedInst = dyn_cast<Instruction>(callInst->getArgOperand(0))) {
            auto searchAssumedInst = branchEnv.find(assumedInst);
            if (searchAssumedInst != branchEnv.end()) {
                // ture env of the branch inst
                curEnv->copyEnvironment(*searchAssumedInst->second.first);
            }
            else {
                // keep the env in which assumedInst var is non-zero
                int zero = 0;
                string assumedVar = getNameFromValue(assumedInst);
                curEnv->performCmpOp(NE, assumedVar, zero);
            }
        }
        return curEnv;
    }

	Environment* checkNonDetIntCall(CallInst *callInst, Environment *curEnv) {
		// errs() << "finding the atomic var to consider\n";
		Value *atomicVarOper = callInst->getOperand(0);
		// printValue(atomicVarOper);
		string atomicVarName = getNameFromValue(atomicVarOper);
		// errs() << "VarName: " << atomicVarName << "\n";
		Value *lbOper = callInst->getOperand(1);
		Value *ubOper = callInst->getOperand(2);
		int lb, ub;
		if (ConstantInt *lbcons = dyn_cast<ConstantInt> (lbOper)) {
			lb = lbcons->getValue().getSExtValue();
		}
		else {
			errs() << "ERROR: start range in nondet_int is not int. Function not supported\n";
			exit(0);
		}
		if (ConstantInt *ubcons = dyn_cast<ConstantInt> (ubOper)) {
			ub = ubcons->getValue().getSExtValue();
		}
		else {
			errs() << "ERROR: start range in nondet_int is not int. Function not supported\n";
			exit(0);
		}
		// errs() << "set to range [" << lb << "," << ub << "]\n";
		curEnv->performNonDetInt(atomicVarName, lb, ub);
		// errs() << "CurEnv after nondet_int:\n";
		// curEnv->printEnvironment();
		return curEnv;
	}

    Environment* checkRMWInst(
        AtomicRMWInst *rmwInst, 
        Environment *curEnv, 
        forward_list<const pair<Instruction*, Instruction*>*>::iterator *curInterfItr,
        forward_list<const pair<Instruction*, Instruction*>*>::const_iterator &endCurInterfItr
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
            curEnv = applyInterfToLoad(rmwInst, curEnv, curInterfItr, endCurInterfItr, pointerVarName);
        }
        // errs() << "After applyInterf:\n";
        // curEnv.printEnvironment();

        // the old value of global is returned
        curEnv->performUnaryOp(LOAD, destVarName.c_str(), pointerVarName.c_str());

        // errs() << "After assigning to the destVar:\n";
        // curEnv.printEnvironment();

        // find the argument to perform RMW with and 
        // update the new value of global variable
        Value *withVar = rmwInst->getValOperand();
        if (ConstantInt *constWithVar = dyn_cast<ConstantInt>(withVar)) {
            int constIntWithVar= constWithVar->getValue().getSExtValue();
            curEnv->performBinaryOp(oper, pointerVarName, pointerVarName, constIntWithVar);
        }
        else {
            string withVarName = getNameFromValue(withVar);
            curEnv->performBinaryOp(oper, pointerVarName, pointerVarName, withVarName);
        }

        // errs() << "After performing binOp:\n";
        // curEnv.printEnvironment();

        // append the current instruction in POMO
        curEnv->performStoreOp(getInstNumByInst(rmwInst), pointerVarName);

        // errs() << "After appending store:\n";
        // curEnv.printEnvironment();
        return curEnv;
    }

    Environment* checkRMWInst(
        AtomicRMWInst *rmwInst, 
        Environment *curEnv, 
        vector<pair<Instruction*, vector<Instruction*>>>::iterator *curInterfItr,
        vector<pair<Instruction*, vector<Instruction*>>>::const_iterator &endCurInterfItr
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
            curEnv = applyInterfToLoad(rmwInst, curEnv, curInterfItr, endCurInterfItr, pointerVarName);
        }
        // errs() << "After applyInterf:\n";
        // curEnv.printEnvironment();

        // the old value of global is returned
        curEnv->performUnaryOp(LOAD, destVarName.c_str(), pointerVarName.c_str());

        // errs() << "After assigning to the destVar:\n";
        // curEnv.printEnvironment();

        // find the argument to perform RMW with and 
        // update the new value of global variable
        Value *withVar = rmwInst->getValOperand();
        if (ConstantInt *constWithVar = dyn_cast<ConstantInt>(withVar)) {
            int constIntWithVar= constWithVar->getValue().getSExtValue();
            curEnv->performBinaryOp(oper, pointerVarName, pointerVarName, constIntWithVar);
        }
        else {
            string withVarName = getNameFromValue(withVar);
            curEnv->performBinaryOp(oper, pointerVarName, pointerVarName, withVarName);
        }

        // errs() << "After performing binOp:\n";
        // curEnv.printEnvironment();

        // append the current instruction in POMO
        curEnv->performStoreOp(getInstNumByInst(rmwInst), pointerVarName);

        // errs() << "After appending store:\n";
        // curEnv.printEnvironment();
        return curEnv;
    }

    Environment* applyInterfToLoad(
        Instruction* loadInst, 
        Environment *curEnv, 
        forward_list<const pair<Instruction*, Instruction*>*>::iterator *curInterfItr,
        forward_list<const pair<Instruction*, Instruction*>*>::const_iterator &endCurInterfItr,
        string varName
    ) {
        // errs() << "Applying interf\n";
        // find interfering instruction
        // auto searchInterf = interf.find(loadInst);
        // if (searchInterf == interf.end()) {
        //     errs() << "ERROR: Interfernce for the load instrction not found\n";
        //     printValue(loadInst);
        //     return curEnv;
        // }
        // fprintf(stderr, "curInterfItr: %p\n", curInterfItr);
        // fprintf(stderr, "*curInterfItr: %p\n", (*curInterfItr));
        if (*curInterfItr == endCurInterfItr) return curEnv;
        auto curLsPair = (**curInterfItr);
        // errs() << "found curLsPair\n";
        if (loadInst != curLsPair->first) {
            errs() << "ERROR: Ordering of load instruction in interf is worng\n";
            errs() << "expected: "; printValue(loadInst);
            errs() << "found: "; printValue(curLsPair->first);
            exit(0);
        }
        Instruction *interfInst = curLsPair->second;
        (*curInterfItr)++;
        
        // if interfernce is from some other thread
        // errs() << "found intrefInst\n";
        if (interfInst && interfInst != loadInst->getPrevNode()) {
            // find the domain of interfering instruction
            auto searchInterfFunc = programState.find(interfInst->getFunction());
            if (searchInterfFunc != programState.end()) {
                // errs() << "Interf env found\n";
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

                    Environment *interfEnv = searchInterfEnv->second;
                    
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
                    if (interfEnv->isModified() || curEnv->isModified())
                        curEnv->applyInterference(varName, *interfEnv, 
                            getInstNumByInst(loadInst), getInstNumByInst(interfInst));
                    else {
                        // errs() << "MOD: not applying interf\n";
                        curEnv->setNotModified();
                    }
                }
            }
            // else errs() << "interf env not found\n";
        }
        return curEnv;
    }

    Environment* applyInterfToLoad(
        Instruction* loadInst, 
        Environment *curEnv, 
        vector<pair<Instruction*, vector<Instruction*>>>::iterator *curInterfItr,
        vector<pair<Instruction*, vector<Instruction*>>>::const_iterator endCurInterfItr,
        string varName
    ) {
        // errs() << "Applying interf\n";
        // find interfering instruction
        // auto searchInterf = interf.find(loadInst);
        // if (searchInterf == interf.end()) {
        //     errs() << "ERROR: Interfernce for the load instrction not found\n";
        //     printValue(loadInst);
        //     return curEnv;
        // }
        // fprintf(stderr, "curInterfItr: %p\n", curInterfItr);
        // fprintf(stderr, "*curInterfItr: %p\n", (*curInterfItr));
        // no interferece left for this thread. Load will be from same context
        if (*curInterfItr == endCurInterfItr) return curEnv;
        // auto curLsPair = (**curInterfItr);
        // errs() << "found curLsPair\n";
        if (loadInst != (*curInterfItr)->first) {
            if(!noPrint) {errs() << "No interf for load\n"; printValue(loadInst);}
            return curEnv;
        }
        Environment predEnv;
		predEnv.copyEnvironment(*curEnv);
        for (auto interfInst=(*curInterfItr)->second.begin(); 
            interfInst!=(*curInterfItr)->second.end(); interfInst++) {
            Environment tmpEnv = predEnv;
            if (!noPrint) {
                errs() << "\nInterf with Store: ";
                (*interfInst)->print(errs());
                errs() << "\n";
            }
            auto searchInterfFunc = programState.find((*interfInst)->getFunction());
            if (searchInterfFunc != programState.end()) {
                // errs() << "Interf env found\n";
                auto searchInterfEnv = searchInterfFunc->second.find(*interfInst);
                if (searchInterfEnv != searchInterfFunc->second.end()) {
                    // apply the interference
                    // errs() << "Before Interf:\n";
                    // curEnv->printEnvironment();
					// errs() << "Interf Env:\n";
					// searchInterfEnv->second->printEnvironment();
					// errs() << "Applying Interf\n";

                    // if (searchInterfEnv->second->isModified() || curEnv->isModified()) {
                        tmpEnv.applyInterference(varName, *searchInterfEnv->second, 
                            getInstNumByInst(loadInst), getInstNumByInst(*interfInst));
						// errs() << "after applying this interf:\n";
						// tmpEnv.printEnvironment();
                    	curEnv->joinEnvironment(tmpEnv);
					// }
                    // else {
                        // errs() << "MOD: not applying interf\n";
						// TODO: get it from prev itr, join with cur.
                        // tmpEnv.setNotModified();
                    // }
					// errs() << "after joining to curEnv:\n";
					// curEnv->printEnvironment();
                }
            }
        }
        (*curInterfItr)++;
        return curEnv;
    }

    // Environment applyInterfToLock(
    //     Instruction* lockInst, 
    //     Environment &curEnv, 
    // ) {
    //     Environment predEnv = curEnv;
    //     for (auto )
    //     for (auto interfInst=(*curInterfItr)->second.begin(); 
    //         interfInst!=(*curInterfItr)->second.end(); interfInst++) {
    //         Environment tmpEnv = predEnv;
    //         if (!noPrint) {
    //             errs() << "\nInterf with Store: ";
    //             (*interfInst)->print(errs());
    //             errs() << "\n";
    //         }
    //         auto searchInterfFunc = programState.find((*interfInst)->getFunction());
    //         if (searchInterfFunc != programState.end()) {
    //             // errs() << "Interf env found\n";
    //             auto searchInterfEnv = searchInterfFunc->second.find(*interfInst);
    //             if (searchInterfEnv != searchInterfFunc->second.end()) {
    //                 // apply the interference
    //                 // errs() << "Before Interf:\n";
    //                 // curEnv.printEnvironment();

    //                 if (searchInterfEnv->second.isModified() || curEnv.isModified())
    //                     tmpEnv.applyInterference(varName, searchInterfEnv->second, 
    //                         getInstNumByInst(loadInst), getInstNumByInst(*interfInst));
    //                 else {
    //                     // errs() << "MOD: not applying interf\n";
    //                     tmpEnv.setNotModified();
    //                 }
    //                 curEnv.joinEnvironment(tmpEnv);
    //             }
    //         }
    //     }
    //     (*curInterfItr)++;
    //     return curEnv;
    // }

    /* void getLoadsToAllStoresMap (
        unordered_map<Function*, forward_list<pair<Instruction*, string>>> &allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> &allStores,
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> &loadsToAllStores
    ){
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
                // errs() << "adding load "; printValue(load);
                loadsToAllStores[curFunc].push_back(make_pair(load, allStoresForCurLoad));
            }
            // errs() << "Load to all stores:\n";
            // for (auto it: (*loadsToAllStores)[curFunc]) {
            //     printValue(it.first);
            // }
        }
        // return loadsToAllStores;
    } */

    void getFeasibleInterferences (
        unordered_map<Function*, forward_list<pair<Instruction*, string>>>  &allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> &allStores,
        const map<Function*, Instruction*> &funcToTCreate,
        const map<Function*, Instruction*> &funcToTJoin,
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> &feasibleInterfences
    ){
        // unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> loadsToAllStores;
        // Make all permutations
        // errs() << "Making all loads to all stores map\n";
        getLoadsToAllStoresMap(allLoads, allStores, feasibleInterfences);
        allLoads.clear();
        allStores.clear();
        // printLoadsToAllStores(feasibleInterfences);
        
        // remove the stores s from l such that either s --ppo--> l or l --ppo--> s
        for (auto funItr=feasibleInterfences.begin(); funItr!=feasibleInterfences.end();) {
            // errs() << "For func: " << funItr->first->getName() << "\n";
            for (auto loadItr=funItr->second.begin(); loadItr!=funItr->second.end(); ) {
                // errs() << "For load: "; printValue(loadItr->first); 
                for (auto storeItr=loadItr->second.begin(); storeItr!=loadItr->second.end();) {
                    if (!(*storeItr) || SBTCreateTJoin(loadItr->first, *storeItr, funcToTCreate, funcToTJoin)) {
                        // interf is infeasible. Remove it
                        // errs() << "Removing "; printValue((*storeItr));
                        storeItr = loadItr->second.erase(storeItr);
                    } else storeItr++;
                }
                if (loadItr->second.empty()) {
                    loadItr = funItr->second.erase(loadItr);
                } else loadItr++;
            }
            if (funItr->second.empty()) {
                funItr = feasibleInterfences.erase(funItr);
            } else funItr++;
        }
        // errs() << "After erasing:\n";
        // printLoadsToAllStores(feasibleInterfences);
    }

    void getLoadsToAllStoresMap (
        unordered_map<Function*, forward_list<pair<Instruction*, string>>> &allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> &allStores,
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> &loadsToAllStores
    ){
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
                // errs() << "adding load "; printValue(load);
                allStoresForCurLoad.push_back(nullptr);
                loadsToAllStores[curFunc].push_back(make_pair(load, allStoresForCurLoad));
            }
            // errs() << "Load to all stores:\n";
            // for (auto it: (*loadsToAllStores)[curFunc]) {
            //     printValue(it.first);
            // }
        }
        // return loadsToAllStores;
    }

    /// Compute all possible interferences (feasibile or infeasible) 
    /// returns maximum number of interferneces in any function
    int getAllInterferences (
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> &loadsToAllStores,
        map<Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> &allInterfs,
        const map<Function*, Instruction*> &funcToTCreate,
        const map<Function*, Instruction*> &funcToTJoin
    ){
        // unordered_map<Function*, vector< map<Instruction*, Instruction*>>> allInterfs;

        // printLoadsToAllStores(loadsToAllStores);
        int maxInterfs = 0;
        
        for (auto funItr=loadsToAllStores.begin(); funItr!=loadsToAllStores.end(); ++funItr) {
            Function *curFunc = funItr->first;
            auto allLS = &(funItr->second);
            Instruction* loads[allLS->size()];
            vector<Instruction*>::iterator allItr[allLS->size()];
            vector<Instruction*>::iterator loadsBeg[allLS->size()];
            vector<Instruction*>::iterator loadsEnd[allLS->size()];
            int noOfInterfs = 1;
            int i=0;
            for (auto itr=allLS->begin(); itr!=allLS->end(); ++itr, i++) {
                loads[i] = itr->first;
                allItr[i] = itr->second.begin();
                loadsBeg[i] = itr->second.begin();
                loadsEnd[i] = itr->second.end();
                if (!itr->second.empty()) noOfInterfs *= itr->second.size();
            }
            if (maxInterfs < noOfInterfs) maxInterfs = noOfInterfs;
            // errs() <<  curFunc->getName() << ": " << noOfInterfs << "\n";

            forward_list< const pair< Instruction*, Instruction* >* > curInterfNew;
            
            for (int k=0; k>=0; ) {
                auto insertPt = curInterfNew.before_begin();
                bool feasible = true;
                int j;
                // errs() << "\ninterf" << i << "\n";
                for (j=0; j<allLS->size(); j++) {
                    if (allItr[j] != loadsEnd[j]) {
                        // Check eager SB to prune out interference
                        // if (l',s') \in insertPt and cur is (l,s),
                        // since loads are ordered acc to sb, 
                        // s --sb--> s' means infeasible interference
                        auto stCur = (*allItr[j]);
                        if (eagerPruning) {
                            // errs() << "LOAD: "; printValue(loads[j]);
                            // errs() << "STORE: "; printValue(stCur);
                            // errs() << "checking SBCreateJoin of\n";
                            if (stCur && SBTCreateTJoin(loads[j], stCur, funcToTCreate, funcToTJoin)) {
                                feasible = false;
                                break;
                            }
                            // errs() << "checking SB with older LS Pairs";
                            for (auto curInterfItr: curInterfNew) {
                                if(curInterfItr->second && stCur && getInstNumByInst(stCur).isSeqBefore(getInstNumByInst(curInterfItr->second))) {
                                    // infeasible. Increment cur st iterator and start new inter
                                    feasible = false;
                                    // errs() << "infeasible\n";
                                    break;
                                }
                            }
                        }
                        if (feasible) {
                            auto lsPairPtr = allLSPairs.insert(make_pair(loads[j], stCur));
                            insertPt = curInterfNew.insert_after(insertPt, &(*lsPairPtr.first));
                        }
                        else break;
                    }
                }
                // int k;
                if (feasible) {
                    k = allLS->size()-1;
                    allInterfs[curFunc].push_back(curInterfNew);
                }
                else k = j; // update the store iterator for infeasible interf
                curInterfNew.resize(0);
                if (allItr[k] != loadsEnd[k]) {
                    allItr[k]++;
                }
                while (k>=0 && allItr[k] == loadsEnd[k]) {
                    allItr[k] = loadsBeg[k];
                    k--;
                    if (k>=0) allItr[k]++;
                }
                // if (k < 0) break;
            }
        }
        loadsToAllStores.clear();
        return maxInterfs;
    }

    /// Checks all possible interfernces for feasibility one by one.
    void getFeasibleInterferences (
        unordered_map<Function*, forward_list<pair<Instruction*, string>>>  &allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> &allStores,
        const map<Function*, Instruction*> &funcToTCreate,
        const map<Function*, Instruction*> &funcToTJoin,
        map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> &feasibleInterfences
    ){
        // map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> *feasibleInterfs = &feasibleInterfences;
        map<Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> allInterfs;
        // map<Function* ,forward_list<InterfNode*>> newAllInterfs;
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> loadsToAllStores;
        // Make all permutations
        // errs() << "Making all loads to all stores map\n";
        getLoadsToAllStoresMap(allLoads, allStores, loadsToAllStores);
        allLoads.clear();
        allStores.clear();
        // double start_time = omp_get_wtime();
        int maxInterfs;
        if (eagerPruning) {
            // errs() << "getting feasible interferences\n";
            maxInterfs = getAllInterferences(loadsToAllStores, feasibleInterfences, funcToTCreate, funcToTCreate);
            return;
        }
        else {
            maxInterfs = getAllInterferences(loadsToAllStores, allInterfs, funcToTCreate, funcToTCreate);
        }
        // getAllInterfsNew(loadsToAllStores, &newAllInterfs);
        // errs() << "Time to compute all interfs: " << (omp_get_wtime() - start_time) << "\n";
        // printAllInterfsNew(&allInterfs);
        // allInterfs = tmp.first; maxInterfs = tmp.second;
        // errs() << "maxInterfs: " << maxInterfs << ", will paralllelize: " << (maxInterfs > 600000) << "\n";

        // errs() << "# of total interference:\n";
        // for (auto it: allInterfs) {
        //     auto tmp = it.second.size();
        //     errs() << it.first->getName() << " : " << tmp << "\n";
        //     if (maxInterfs < tmp) maxInterfs = tmp;
        // }

        // errs() << "Interfs:\n";
        // for (auto funcIt: allInterfs) {
        //     errs() << funcIt.first->getName() << " :\n";
        //     int i = 0; 
        //     for (auto vecIt: funcIt.second) {
        //         errs() << "Interf " << i << ":\n";
        //         for (auto listIt: vecIt) {
        //             printValue(listIt->first); errs() << "--->";
        //             printValue(listIt->second); errs() << "\n";
        //         }
        //         i++;
        //         errs() << "\n";
        //     }
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

        // #pragma omp parallel num_threads(threads.size()) if (maxInterfs > 300)
        // #pragma omp single 
        // {
        // #pragma omp parallel for //shared(feasibleInterfences, minimalZ3,funcToTCreate,funcToTJoin) num_threads(allInterfs.size())
        for (auto funcItr=allInterfs.begin(); funcItr!=allInterfs.end(); funcItr++) {
            // #pragma omp task if (funcItr->second.size() > 300) \
            // shared(minimalZ3, instToNum, numToInst, funcToTCreate,funcToTJoin) firstprivate(funcItr)
            // {
            // errs() << "TID: " << omp_get_thread_num() << ", func: " << funcItr.first->getName() << "\n";
            // vector< forward_list<const pair<Instruction*, Instruction*>*>> curFuncInterfs;
            int i = 0;
            for (auto interfItr=funcItr->second.begin(); interfItr!=funcItr->second.end(); i++,interfItr++) {
                // auto interfs = *interfItr;
                // errs() << "checking " << i << "\n" ;
                // for (auto listIt: (*interfItr)) {
                //     printValue(listIt->first); errs() << "--->";
                //     printValue(listIt->second); errs() << "\n";
                // }
                // errs() << "\n";
                // if (minimalZ3) {
                //     if(!isFeasibleRA(&(*interfItr)))
                //         interfItr =  funcItr->second.erase(interfItr);
                //         // curFuncInterfs.push_back(interfItr);
                //     else interfItr++;
                // }
                // else {
                    bool isFeasible = isFeasibleRAWithoutZ3(&(*interfItr), funcToTCreate, funcToTJoin);
                    if (isFeasible) {
                        feasibleInterfences[funcItr->first].push_back(*interfItr);
                    }
                    // if (!isFeasible) {
                    //     // errs() << "deleting " << i << "\n";
                    //     // for (auto listItr: (*interfItr)) {
                    //     //     printValue(listItr->first); errs() << "--->";
                    //     //     printValue(listItr->second); errs() << "\n";
                    //     // }
                    //     interfItr = funcItr->second.erase(interfItr);
                    //     // errs() << "deleted\n";
                    // }
                    // else ++interfItr;
                    // if (isFeasible) curFuncInterfs.push_back(interfItr);
                // } 
            }
            // allInterfs[funcItr->first].clear();
            // funcItr->second.shrink_to_fit();
            // }
        }
        // }
        // feasibleInterfences = allInterfs;
    }

    /// Older function. Used with UseZ3 flag.
    #ifdef NOTRA 
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
    #endif

    /** This function checks the feasibility of an interference combination 
     * under RA memory model 
     */
    bool isFeasibleRAWithoutZ3(const forward_list<const pair<Instruction*, Instruction*>*> *interfs,
        const map<Function*, Instruction*> &funcToTCreate,
        const map<Function*, Instruction*> &funcToTJoin
    ) {
        for (auto lsPair : (*interfs)) {
            if (lsPair->second == nullptr)
                continue;

            // if (isSeqBefore(lsPair->first, lsPair->second)) return false;
            if (SBTCreateTJoin(lsPair->first, lsPair->second, funcToTCreate, funcToTJoin)) return false;
            if (!eagerPruning) {
                for (auto otherLS : (*interfs)) {
                    // lsPair: (s --rf--> l), otherLS: (s' --rf--> l')
                    if (otherLS == lsPair || otherLS->second==nullptr)
                        continue;
                    Instruction *ld = lsPair->first;
                    Instruction *st = lsPair->second;
                    Instruction *ld_prime = otherLS->first;
                    Instruction *st_prime = otherLS->second;
                    // (l --sb--> l')
                    if (getInstNumByInst(ld).isSeqBefore(getInstNumByInst(ld_prime))) {
                        // (l --sb--> l' && s = s') reading from local context will give the same result
                        if (st == st_prime) return false;
                        else if (getInstNumByInst(st_prime).isSeqBefore(getInstNumByInst(st))) 
                            return false;
                    }
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
        const map<Function*, Instruction*> &funcToTCreate,
        const map<Function*, Instruction*> &funcToTJoin
    ) {
		// errs() << "SBTCreateTJoin called with ld: "; printValue(ld);
		// errs() << "st: "; printValue(st);
		// errs() << "funcToTCreate:\n";
		// for (auto it=funcToTCreate.begin(); it!=funcToTCreate.end(); it++) {
		// 	errs() << it->first->getName() << " : "; printValue(it->second);
		// }
		// errs() << "funcToTJoin:\n";
		// for (auto it=funcToTJoin.begin(); it!=funcToTJoin.end(); it++) {
		// 	errs() << it->first << " : "; printValue(it->second);
		// }

        Function* ldFunc = ld->getFunction();
        // lab: tcreate(f) /\ l in f /\ s--sb-->lab ==> s--nrf-->l
        auto searchLdFuncTC = funcToTCreate.find(ldFunc);
        if (searchLdFuncTC != funcToTCreate.end()) {
            if(getInstNumByInst(st).isSeqBefore(getInstNumByInst(searchLdFuncTC->second))) 
                return true;
        }
        else {
            // since no tcreate instruction is found, the function name must be main
            assert(ldFunc->getName()=="main" && "no tcreate of function of load inst found and load is not in main");
        }

        // lab: tjoin(f) /\ l in f /\ lab--sb-->s ==> s--nrf-->l
        auto searchldFuncTJ = funcToTJoin.find(ldFunc);
        if (searchldFuncTJ != funcToTJoin.end()) {
            if(getInstNumByInst(searchldFuncTJ->second).isSeqBefore(getInstNumByInst(st))) 
                return true;
        }
        else {
            // since no tjoin instruction is found, the function name must be main
            assert(ldFunc->getName()=="main" && "no tjoin of function of load inst found and load is not in main");
        }

        Function* stFunc = st->getFunction();
        // lab: tcreate(f) /\ s in f /\ l--sb-->lab ==> s--nrf-->l
        auto searchStFucnTC = funcToTCreate.find(stFunc);
        if (searchStFucnTC != funcToTCreate.end()) {
            if(getInstNumByInst(ld).isSeqBefore(getInstNumByInst(searchStFucnTC->second))) 
                return true;
        }
        else {
            // since no tcreate instruction is found, the function name must be main
            assert(stFunc->getName()=="main" && "no tcreate of function of store inst found and store is not in main");
        }

        // lab: tjoin(f) /\ s in f /\ lab--sb-->l ==> s--nrf-->l
        auto searchStFuncTJ = funcToTJoin.find(stFunc);
        if (searchStFuncTJ != funcToTJoin.end()) {
            if(getInstNumByInst(searchStFuncTJ->second).isSeqBefore(getInstNumByInst(ld))) 
                return true;
        }
        else {
            // since no tjoin instruction is found, the function name must be main
            assert(stFunc->getName()=="main" && "no tjoin of function of store ist found and store is not in main");
        }
        return false;
    }

    unordered_map<Instruction*, Environment*> joinEnvByInstruction (
        unordered_map<Instruction*, Environment*> instrToEnvOld,
        unordered_map<Instruction*, Environment*> instrToEnvNew
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
                Environment *newEnv = searchNewMap->second;
                newEnv->joinEnvironment(*itOld->second);
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
                        Environment *curEnv = it->second;
						// curEnv->printEnvironment();
                        if (!curEnv->isUnreachable()) {
                            errs() << "Assertion failed:\n";
                            printValue(callInst);
                            if (!noPrint)
                                curEnv->printEnvironment();
                            num_errors++;
                        }
                    }
                }
            }
        }
        errs() << "___________________________________________________\n";
        errs() << "# Failed Asserts: " << num_errors << "\n";
    }

    void printLoadsToAllStores(
        unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> loadsToAllStores
    ){
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

    void printInstToEnvMap(unordered_map<Instruction*, Environment*> instToEnvMap) {
        for (auto it=instToEnvMap.begin(); it!=instToEnvMap.end(); ++it) {
            it->first->print(errs());
            if (it->second) it->second->printEnvironment();
			else errs() << "NULL\n";
        }
    }

    void printProgramState(unordered_map <Function*, unordered_map<Instruction*, Environment*>> newProgramState) {
        for (auto it1=newProgramState.begin(); it1!=newProgramState.end(); ++it1) {
            errs() << "\n-----------------------------------------------\n";
            errs() << "Function " << it1->first->getName() << ":\n";
            errs() << "-----------------------------------------------\n";
            printInstToEnvMap(it1->second);
        }
    }

	bool isEqualFuncEnv(
		unordered_map<Instruction*, Environment*> oldState, 
		unordered_map<Instruction*, Environment*> newState
	) {
		for (auto it=newState.begin(); it!=newState.end(); it++) {
			// errs() << "inst: "; printValue(it->first);
			// errs() << "new state:\n";
			// it->second->printEnvironment();
			auto searchInstEnv = oldState.find(it->first);
			if (searchInstEnv == oldState.end()) {
				return false;
			}
			// errs() << "oldState:\n";
			// searchInstEnv->second->printEnvironment();
			if (!(*(it->second) == *(searchInstEnv->second))) {
				return false;
			}
		}
		return true;
	}

    bool isFixedPoint(unordered_map <Function*, unordered_map<Instruction*, Environment*>> newProgramState) {
        // if (iterations == 4) {
        //      errs() << "checking fixed point...\n";
        //      errs() << "old state:\n"; printProgramState(programState);
        //      errs() << "new program state:\n"; printProgramState(newProgramState);    
        // }
        // errs() << "Program state size: " << programState.size();
        // errs() << "\nnew program state size: " << newProgramState.size()<<"\n";
		for (auto it=newProgramState.begin(); it!=newProgramState.end(); it++) {
			auto searchFunEnv = programState.find(it->first);
			if (searchFunEnv == programState.end()) {
				return false;
			}
			if (!isEqualFuncEnv(it->second, searchFunEnv->second))
				return false;
			// errs() << "****Env of thread " << it->first->getName() << " is same**\n";
		}
        // return (newProgramState == programState);
		return true;
    }

	void clearProgramState(unordered_map <Function*, unordered_map<Instruction*, Environment*>> &programState) {
		for (auto it=programState.begin(); it!=programState.end(); it++) {
			clearFuncProgramState(it->second);
		}
		programState.clear();
	}

	void clearFuncProgramState(unordered_map<Instruction*, Environment*> &funcProgramState) {
		for (auto it=funcProgramState.begin(); it!=funcProgramState.end(); it++) {
			delete it->second;
		}
		funcProgramState.clear();
	}

    void countNumFeasibleInterf (const map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> &feasibleInterfences
    ) {
        errs() << "# of feasible interference:\n";
        maxFeasibleInterfs = 0;
        for (auto it: feasibleInterfences) {
            if (it.second.size()>maxFeasibleInterfs)
                maxFeasibleInterfs = it.second.size();
            errs() << it.first->getName() << " : " << it.second.size() << "\n";
        }
    }

    void countNumFeasibleInterf (const unordered_map<Function*, vector<pair<Instruction*, vector<Instruction*>>>> feasibleInterfences) {
        // errs() << "Number of feasible interferences per thread" << 
        //         "Format: (total, per load interferences, max interfs\n";
        errs() << "num_interfs: ";
        for (auto it: feasibleInterfences) {
            errs() << it.first->getName();
            int num_loads=0;
            int max=0;
            int total=0;
            for (auto it2: it.second) {
                int interfcount = it2.second.size();
                total += interfcount;
                num_loads++;
                if (interfcount > max) max = interfcount;
            }
            errs() << "(" << total << "," << (total/num_loads) << "," 
                    <<  max << ")\n";
        }
    }

    void printFeasibleInterf(const map <Function*, vector< forward_list<const pair<Instruction*, Instruction*>*>>> &feasibleInterfences
    ) {
        errs() << "\nFeasible Interfs\n";
        for (auto it1=feasibleInterfences.begin(); it1!=feasibleInterfences.end(); ++it1) {
            errs() << "Interfs for function: " << it1->first->getName() << "\n";
            // auto allInterfOfFun = it1->second;
            int i = 0;
            // iterate over al interfs of current fnctaion
            for (auto it2=it1->second.begin(); it2!=it1->second.end(); ++it2, ++i) {
                errs() << "Interf " << i << ":\n";
                // print the interf pairs
                for (auto it3=it2->begin(); it3!=it2->end(); ++it3) {
                    errs() << "\tLoad: ";
                    (*it3)->first->print(errs());
                    errs() << "\n\tStore: ";
                    if ((*it3)->second) (*it3)->second->print(errs());
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
                ! isPthreadJoin(prevInst) 	&&
				! isLockInst(prevInst)		&&
				! isUnlockInst(prevInst)) {
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
                ! isPthreadJoin(nextInst)	&&
				! isLockInst(nextInst)		&&
				! isUnlockInst(nextInst)) {
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
            // fprintf(stderr, "%s: ", it.second.toString());
            errs() << it.second.toString() << ": " << it.first << ": ";
            printValue(it.first);
            // errs() << "\t" << it.second.toString() << "\n";
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
