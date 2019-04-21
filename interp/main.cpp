#include "common.h"
#include "domain.h"
#include "analyzer.h"
#include "z3_handler.h"

class VerifierPass : public ModulePass {

    /* TODO: Should I create threads as a set or vector??
        Vector- interferences of t1 with t1 will be explored in case more than one thread of same func are present
        Set and interference of each thread with itself are also explored, support to inf threads of same func can be added 
    */
    vector <Function*> threads;
    unordered_map <Function*, unordered_map<Instruction*, Environment>> programState;
    unordered_map <Function*, Environment> funcInitEnv;
    unordered_map <Function*, vector< unordered_map<Instruction*, Instruction*>>> feasibleInterfences;
    unordered_map <string, Value*> nameToValue;
    unordered_map <Value*, string> valueToName;
    Z3Helper zHelper;

    
    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        // TODO: get domain type based on comman line arguments
        string domainType = "box";
        
        // zHelper.testFixedPoint();

        vector<string> globalVars = getGlobalIntVars(M);
        
        zHelper.initZ3(globalVars);

        initThreadDetails(M, globalVars, domainType);

        // testApplyInterf();

        analyzeProgram(M);

        // unsat_core_example1();
        errs() << "----DONE----\n";
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
                // Push the init to read from self envionment
                allStoresForCurLoad.push_back(nullptr);
                loadsToAllStores[curFunc][load] = allStoresForCurLoad;
            }
        }
        // printLoadsToAllStores(loadsToAllStores);

        return loadsToAllStores;
    }

    void initThreadDetails(Module &M, vector<string> globalVars, string domainType) {
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

            for(auto block = func->begin(); block != func->end(); block++)          //iterator of Function class over BasicBlock
            {
                Instruction *lastGlobalInst=nullptr;
                unordered_map<string, unordered_set<Instruction*>> varToStores;
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
                                    // need to add dominates rules
                                    Instruction *lastGlobalInstBeforeCall = getLastGlobalInst(call);
                                    Instruction *nextGlobalInstAfterCall  = getNextGlobalInst(call->getNextNode());
                                    Instruction *firstGlobalInstInCalled  = getNextGlobalInst(&*(newThread->begin()->begin()));
                                    // lastGlobalInstBeforeCall (or firstGlobalInstInCalled) == nullptr means there 
                                    // no global instr before thread create in current function (or in newly created thread)
                                    if (lastGlobalInstBeforeCall) {
                                        zHelper.addMHB(lastGlobalInstBeforeCall, call);
                                        relations.push_back(make_pair("mhb", make_pair(lastGlobalInstBeforeCall, call)));
                                    }
                                    if (nextGlobalInstAfterCall) {
                                        zHelper.addMHB(call, nextGlobalInstAfterCall);
                                        relations.push_back(make_pair("mhb", make_pair(call, nextGlobalInstAfterCall)));
                                    }
                                    if (firstGlobalInstInCalled) {
                                        zHelper.addMHB(call, firstGlobalInstInCalled);
                                        relations.push_back(make_pair("mhb", make_pair(call, firstGlobalInstInCalled)));
                                    }
                                    if (lastGlobalInst) {
                                        zHelper.addPO(lastGlobalInst, call);
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
                                zHelper.addMHB(call, nextGlobalInstAfterCall);
                                relations.push_back(make_pair("mhb", make_pair(call, nextGlobalInstAfterCall)));
                            }
                            if (lastGlobalInstBeforeCall) {
                                zHelper.addMHB(lastGlobalInstBeforeCall, call);
                                relations.push_back(make_pair("mhb", make_pair(lastGlobalInstBeforeCall, call)));
                            }
                            for (auto it=lastGlobalInstInCalled.begin(); it!=lastGlobalInstInCalled.end(); ++it) {
                                zHelper.addMHB(*it, call);
                                relations.push_back(make_pair("mhb", make_pair(*it, call)));
                            }
                            if (lastGlobalInst) {
                                zHelper.addPO(lastGlobalInst, call);
                                relations.push_back(make_pair("po", make_pair(lastGlobalInst, call)));
                            }
                        }
                        else {
                            errs() << "unknown function call:\n";
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
                            // errs() << "****adding store instr for: ";
                            // storeInst->print(errs());
                            // errs() << "\n";
                            zHelper.addStoreInstr(storeInst);
                            if (lastGlobalInst) {
                                // zHelper.addPO(lastGlobalInst, storeInst);
                                relations.push_back(make_pair("po", make_pair(lastGlobalInst, storeInst)));
                            } 
                            // no global operation yet. Add MHB with init
                            else {
                                relations.push_back(make_pair("mhb", make_pair(lastGlobalInst, storeInst)));
                            }
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
                                varToLoads.emplace(loadInst, fromVarName);
                                // errs() << "****adding load instr for: ";
                                // printValue(loadInst);
                                zHelper.addLoadInstr(loadInst);
                                if (lastGlobalInst) {
                                    // Helper.addPO(lastGlobalInst, loadInst);
                                    relations.push_back(make_pair("po", make_pair(lastGlobalInst, loadInst)));
                                }
                                // no global operation yet. Add MHB with init
                                else {
                                    relations.push_back(make_pair("mhb", make_pair(lastGlobalInst, loadInst)));
                                }
                                lastGlobalInst = loadInst;
                            }
                        }
                    }
                }

                // Save loads stores function wise
                allStores.emplace(func, varToStores);
                allLoads.emplace(func, varToLoads);

            }
            Environment curFuncEnv;
            curFuncEnv.init(domainType, globalVars, funcVars);
            funcInitEnv[func] = curFuncEnv;
        }
        getFeasibleInterferences(allLoads, allStores, relations);
    }

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unsigned int iterations = 0;
        unordered_map <Function*, unordered_map<Instruction*, Environment>> programStateCurItr;
        bool isFixedPointReached = false;

        while (!isFixedPointReached) {
            programState = programStateCurItr;
            
            errs() << "_________________________________________________\n";
            errs() << "Iteration: " << iterations << "\n";

            for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
                Function *curFunc = (*funcItr);
                fprintf(stderr, "\n******DEBUG: Analyzing thread %s*****\n", curFunc->getName());

                // find feasible interfernce for current function
                vector <unordered_map <Instruction*, Instruction*>> curFuncInterfs;
                unordered_map<Instruction*, Environment> newFuncEnv;

                auto searchInterf = feasibleInterfences.find(curFunc);
                
                if (searchInterf != feasibleInterfences.end()) {
                    curFuncInterfs = (searchInterf->second);
                } else {
                    errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        unordered_map <Instruction*, Instruction*> interf;
                        newFuncEnv = analyzeThread(*funcItr, interf);
                        programStateCurItr.emplace(curFunc, newFuncEnv);
                    }
                }
                
                // errs() << "Number of interf= " << curFuncInterfs.size();
                // analyze the Thread for each interference
                for (auto interfItr=curFuncInterfs.begin(); interfItr!=curFuncInterfs.end(); ++interfItr){
                    // errs() << "\n***For new interf\n";

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
            
            isFixedPointReached = isFixedPoint(programStateCurItr);
            iterations++;
            // printProgramState();
        }
        errs() << "_________________________________________________\n";
        errs() << "Fized point reached in " << iterations << " iteratons\n";
        errs() << "Final domain:\n";
        printProgramState();

    }

    unordered_map<Instruction*, Environment> analyzeThread (Function *F, unordered_map<Instruction*, Instruction*> interf) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, Environment> curFuncEnv;
        curFuncEnv[&(*(F->begin()->begin()))] = funcInitEnv[F];

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
                curEnv = checkBinInstruction(binOp, curEnv);
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
                    errs() << "\nTrue Branch:\n";
                    printValue(trueBranch);
                    errs() << "True branch Env:\n";
                    curFuncEnv[trueBranch].printEnvironment();
                    errs() << "\nFalse Branch:\n";
                    printValue(falseBranch);
                    errs() << "False branch Env:\n";
                    curFuncEnv[falseBranch].printEnvironment();
                }
                else {
                    Instruction *successors = &(*(branchInst->getSuccessor(0)->begin()));
                    curFuncEnv[successors].joinEnvironment(curEnv);
                }
            }
            else if (CallInst *callInst = dyn_cast<CallInst>(instItr)) {
                if (callInst->getCalledFunction()->getName() == "__assert_fail") {
                    errs() << "*** found assert" << "\n";
                    printValue(callInst);
                    if (!curEnv.isUnreachable()) {
                        errs() << "ERROR: Assertion failed\n";
                        printValue(callInst);
                        exit(0);
                    }
                }
            }
            else {
                
            }
            // RMW, CMPXCHG

            curFuncEnv[currentInst] = curEnv;
            predEnv.copyEnvironment(curEnv);
        }
           
        return curEnv;
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

    Environment checkCmpInst(
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
                errs() << "WARNING: Unknown cmp instruction: ";
                cmpInst->print(errs());
                errs() << "\n";
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
        
        branchEnv[cmpInst] = make_pair(trueBranchEnv, falseBranchEnv);
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
            case Instruction::And:
                oper = LAND;
                break;
            case Instruction::Or:
                oper = LOR;
                break;
            // TODO: add more cases
            default:
                fprintf(stderr, "WARNING: unknown binary operation: ");
                printValue(binOp);
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
            curEnv.changeRelHeadIfNull(destVarName, storeInst);
        }
        else {
            curEnv.changeRelHeadToNull(destVarName, storeInst);
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
                    curEnv = applyInterfToLoad(unaryInst, curEnv, interf, fromVarName);
                }
                break;
            // TODO: add more cases
            default: 
                fprintf(stderr, "ERROR: unknown operation: ");
                unaryInst->print(errs());
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
                // errs() << "\nInterf with Store: ";
                // interfInst->print(errs());
                // errs() << "\n";
                if (searchInterfEnv != searchInterfFunc->second.end()) {
                    // apply the interference
                    // errs() << "Before Interf:\n";
                    // curEnv.printEnvironment();

                    // TODO: need to set the bool value proplerly
                    Environment interfEnv = searchInterfEnv->second;
                    bool isRelSeq = false;
                    if (StoreInst *storeInst = dyn_cast<StoreInst>(interfInst)) {
                        if (LoadInst *loadInst = dyn_cast<LoadInst>(unaryInst)) {
                            auto ordStore = storeInst->getOrdering();
                            auto ordLoad  = loadInst->getOrdering();
                            if (ordLoad==llvm::AtomicOrdering::Acquire || 
                                    ordLoad==llvm::AtomicOrdering::SequentiallyConsistent ||
                                    ordLoad==llvm::AtomicOrdering::AcquireRelease) {
                                // Instruction *relHead = interfApDomain.getRelHead(varName);
                                // if (relHead != nullptr) {
                                //     curEnv.setRelHead(varName, relHead);
                                    isRelSeq = true;
                                // }
                            }
                        }
                    }
                    curEnv.applyInterference(varName, interfEnv, isRelSeq);
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

    unordered_map<Function*, vector< unordered_map<Instruction*, Instruction*>>> getFeasibleInterferences (
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads,
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores, 
        vector< pair <string, pair<Instruction*, Instruction*>>> relations
    ){
        unordered_map<Function*, vector< unordered_map<Instruction*, Instruction*>>> allInterfs;
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores;
        // Make all permutations
        // TODO: add dummy env i.e. load from itself
        loadsToAllStores = getLoadsToAllStoresMap(allLoads, allStores);
        allInterfs = getAllInterferences(loadsToAllStores);

        // TODO: Check feasibility of permutations and save them in feasibleInterfences
        for (auto funcItr=allInterfs.begin(); funcItr!=allInterfs.end(); ++funcItr) {
            vector< unordered_map<Instruction*, Instruction*>> curFuncInterfs;
            for (auto interfItr=funcItr->second.begin(); interfItr!=funcItr->second.end(); ++interfItr) {
                if (isFeasible(*interfItr, allLoads, allStores, relations)) {
                    curFuncInterfs.push_back(*interfItr);
                }
            }
            feasibleInterfences[funcItr->first] = curFuncInterfs;
        }
        // printFeasibleInterf();

        return feasibleInterfences;
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
        //     errs() << "\t\t--Reads from-->\t";
        //     if (it->second == nullptr) errs() << "init";
        //     else it->second->print(errs());
        //     errs() << "\n";
        // }

        // Z3Helper checker;
        // checker.addInferenceRules();
        // checker.addMHBandPORules(relations);
        // checker.addAllLoads(allLoads);
        // checker.addAllStores(allStores);
        // return checker.checkInterference(interfs);
        
        // checker.testQuery();
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

    void printLoadsToAllStores(unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores){
        errs() << "All load-store pair in the program\n";
        for (auto it1=loadsToAllStores.begin(); it1!=loadsToAllStores.end(); ++it1) {
            errs () << "***Function " << it1->first->getName() << ":\n";
            auto l2s = it1->second;
            for (auto it2=l2s.begin(); it2!=l2s.end(); ++it2) {
                errs() << "Stores for Load: ";
                it2->first->print(errs());
                errs() << "\n";
                auto stores = it2->second;
                for (auto it3=stores.begin(); it3!=stores.end(); ++it3) {
                    errs() << "\t";
                    (*it3)->print(errs());
                    errs() << "\n";
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
        fun2Env.applyInterference("x", fun1Env, true);
        errs() << "After applying:\n";
        fun2Env.printEnvironment();
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
        errs() << "\nAll Interfs\n";
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
                    else errs() << "INIT";
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
        val->print(errs());
        errs() << "\n";
    }

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true);
