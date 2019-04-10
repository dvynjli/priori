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
    unordered_map <Function*, unordered_map<Instruction*, ApDomain>> programState;
    unordered_map <Function*, ApDomain> funcInitApDomain;
    unordered_map <Function*, vector< unordered_map<Instruction*, Instruction*>>> feasibleInterfences;
    unordered_map <string, Value*> nameToValue;
    unordered_map <Value*, string> valueToName;
    Z3Helper zHelper;

    
    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        // ApDomain initApDomain = ApDomain();
        // TODO: get domain type based on comman line arguments
        string domainType = "box";
        
        // zHelper.testFixedPoint();

        vector<string> globalVars = getGlobalIntVars(M);
        initThreadDetails(M, globalVars, domainType);

        // testApplyInterf();

        zHelper.initZ3(globalVars);

        analyzeProgram(M);

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
                // Push the prev instruction to read from self envionment
                allStoresForCurLoad.push_back(load->getPrevNode());
                loadsToAllStores[curFunc][load] = allStoresForCurLoad;
            }
        }
        // printLoadsToAllStores(loadsToAllStores);

        return loadsToAllStores;
    }

    void initThreadDetails(Module &M, vector<string> globalVars, string domainType) {
        unordered_map<Function*, unordered_map<Instruction*, string>> allLoads;
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores;

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
            for(auto block = func->begin(); block != func->end(); block++)          //iterator of Function class over BasicBlock
            {
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
                                    }
                                    if (nextGlobalInstAfterCall) {
                                        zHelper.addMHB(call, nextGlobalInstAfterCall);
                                    }
                                    if (firstGlobalInstInCalled) {
                                        zHelper.addMHB(call, firstGlobalInstInCalled);
                                    }
                                }

                            }
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
                            Instruction *lastGlobalInstBeforeCall = getLastGlobalInst(call);
                            Instruction *nextGlobalInstAfterCall  = getNextGlobalInst(call->getNextNode());
                            vector<Instruction*> lastGlobalInstInCalled = getLastInstOfJoin(call);
                            if (nextGlobalInstAfterCall) {
                                zHelper.addMHB(call, nextGlobalInstAfterCall);
                            }
                            if (lastGlobalInstBeforeCall) {
                                zHelper.addMHB(lastGlobalInstBeforeCall, call);
                            }
                            for (auto it=lastGlobalInstInCalled.begin(); it!=lastGlobalInstInCalled.end(); ++it) {
                                zHelper.addMHB(*it, call);
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
                                varToLoads.emplace(loadInst, fromVarName);
                                // errs() << "****adding load instr for: ";
                                // printValue(loadInst);
                                zHelper.addLoadInstr(loadInst);
                            }
                        }
                    }
                }

                // Save loads stores function wise
                allStores.emplace(func, varToStores);
                allLoads.emplace(func, varToLoads);

            }
            ApDomain curFuncApDomain;
            curFuncApDomain.init(domainType, globalVars, funcVars);
            funcInitApDomain.emplace(func, curFuncApDomain);
        }
        getFeasibleInterferences(allLoads, allStores);
    }

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unsigned int iterations = 0;
        unordered_map <Function*, unordered_map<Instruction*, ApDomain>> programStateCurItr;
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
                unordered_map<Instruction*, ApDomain> newFuncApDomain;

                auto searchInterf = feasibleInterfences.find(curFunc);
                
                if (searchInterf != feasibleInterfences.end()) {
                    curFuncInterfs = (searchInterf->second);
                } else {
                    errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        unordered_map <Instruction*, Instruction*> interf;
                        newFuncApDomain = analyzeThread(*funcItr, interf);
                        programStateCurItr.emplace(curFunc, newFuncApDomain);
                    }
                }
                
                // errs() << "Number of interf= " << curFuncInterfs.size();
                // analyze the Thread for each interference
                for (auto interfItr=curFuncInterfs.begin(); interfItr!=curFuncInterfs.end(); ++interfItr){
                    // errs() << "\n***For new interf\n";

                    newFuncApDomain = analyzeThread(*funcItr, *interfItr);

                    // errs() << "ApDomain after analysis:\n";
                    // printInstToApDomainMap(newFuncApDomain);

                    // join newFuncApDomain of all feasibleInterfs and replace old one in state
                    auto searchFunApDomain = programStateCurItr.find(curFunc);
                    if (searchFunApDomain == programStateCurItr.end()) {
                        // errs() << "curfunc not found in program state\n";
                        programStateCurItr.emplace(curFunc, newFuncApDomain);
                    }
                    else {
                        // errs() << "curfunc already exist in program state. joining\n";
                        programStateCurItr[curFunc] =  joinApDomainByInstruction(searchFunApDomain->second, newFuncApDomain);
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

    unordered_map<Instruction*, ApDomain> analyzeThread (Function *F, unordered_map<Instruction*, Instruction*> interf) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, ApDomain> curFuncApDomain;

        for(auto bbItr=F->begin(); bbItr!=F->end(); ++bbItr){
            BasicBlock *currentBB = &(*bbItr);

            ApDomain predApDomain = funcInitApDomain[F];
            // predApDomain.printApDomain();
            // initial domain of pred of cur bb to join of all it's pred
            for (BasicBlock *Pred : predecessors(currentBB)){
                auto searchPredBBApDomain = curFuncApDomain.find(Pred->getTerminator());
                if (searchPredBBApDomain != curFuncApDomain.end())
                    predApDomain.joinApDomain(searchPredBBApDomain->second);
                // TODO: if the domain is empty? It means pred bb has not been analyzed so far
                // we can't analyze current BB
            }
            // if termination statement
                // if coditional branching
                // if unconditional branching

            predApDomain = analyzeBasicBlock(currentBB, predApDomain, curFuncApDomain, interf);
        }

        // errs() << "\nApDomain of function:\n";
        // printInstToApDomainMap(curFuncApDomain);

        return curFuncApDomain;
    }

    ApDomain analyzeBasicBlock (BasicBlock *B, 
        ApDomain predApDomain, 
        unordered_map <Instruction*, ApDomain> &curFuncApDomain,
        unordered_map<Instruction*, Instruction*> interf
    ) {
        // check type of inst, and performTrasformations
        ApDomain curApDomain;
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);
            curApDomain.copyApDomain(predApDomain);
        
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
                curApDomain = checkBinInstruction(binOp, curApDomain);
            }
            else if (StoreInst *storeInst = dyn_cast<StoreInst>(instItr)) {
                curApDomain = checkStoreInst(storeInst, curApDomain);
            }
            else if (UnaryInstruction *unaryInst = dyn_cast<UnaryInstruction>(instItr)) {
                curApDomain = checkUnaryInst(unaryInst, curApDomain, interf);
            }
            else if (CmpInst *cmpInst = dyn_cast<CmpInst> (instItr)) {
                errs() << "cmpInst: ";
                cmpInst->print(errs());
                errs() << "\n";
                curApDomain = checkCmpInst(cmpInst, curApDomain);
                
            }
            else {
                
            }
            // RMW, CMPXCHG

            curFuncApDomain[currentInst] = curApDomain;
            predApDomain.copyApDomain(curApDomain);
        }
        
        
        return curApDomain;
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

    ApDomain checkCmpInst(CmpInst* cmpInst, ApDomain curApDomain) { 
        // need to computer ApDomain of both true and false branch
        ApDomain trueBranchApDomain;
        ApDomain falseBranchApDomain;
        trueBranchApDomain.copyApDomain(curApDomain);
        falseBranchApDomain.copyApDomain(curApDomain);

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
                return curApDomain;
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
                trueBranchApDomain.performCmpOp(operTrueBranch, constFromIntVar1, constFromIntVar2);
                falseBranchApDomain.performCmpOp(operFalseBranch, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                trueBranchApDomain.performCmpOp(operTrueBranch, constFromIntVar1, fromVar2Name);
                falseBranchApDomain.performCmpOp(operFalseBranch, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            trueBranchApDomain.performCmpOp(operTrueBranch, fromVar1Name, constFromIntVar2);
            falseBranchApDomain.performCmpOp(operFalseBranch, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            trueBranchApDomain.performCmpOp(operTrueBranch, fromVar1Name, fromVar2Name);
            falseBranchApDomain.performCmpOp(operFalseBranch, fromVar1Name, fromVar2Name);
        }

        return trueBranchApDomain;
    }

    //  call approprproate function for the inst passed
    ApDomain checkBinInstruction(BinaryOperator* binOp, ApDomain curApDomain) {
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
                return curApDomain;
        }

        string destVarName = getNameFromValue(binOp);
        Value* fromVar1 = binOp->getOperand(0);
        Value* fromVar2 = binOp->getOperand(1);
        if (ConstantInt *constFromVar1 = dyn_cast<ConstantInt>(fromVar1)) {
            int constFromIntVar1= constFromVar1->getValue().getSExtValue();
            if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
                int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
                curApDomain.performBinaryOp(oper, destVarName, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                curApDomain.performBinaryOp(oper, destVarName, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            curApDomain.performBinaryOp(oper, destVarName, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            curApDomain.performBinaryOp(oper, destVarName, fromVar1Name, fromVar2Name);
        }

        return curApDomain;
    }

    ApDomain checkStoreInst(StoreInst* storeInst, ApDomain curApDomain) {
        Value* destVar = storeInst->getPointerOperand();
        string destVarName = getNameFromValue(destVar);

        auto ord = storeInst->getOrdering();
        if (ord==llvm::AtomicOrdering::Release || 
                ord==llvm::AtomicOrdering::SequentiallyConsistent ||
                ord==llvm::AtomicOrdering::AcquireRelease) {
            // if (curApDomain.getRelHead(destVarName) == nullptr)
            //     curApDomain.setRelHead(destVarName, storeInst);
        }

        Value* fromVar = storeInst->getValueOperand();
        
        if (ConstantInt *constFromVar = dyn_cast<ConstantInt>(fromVar)) {
            int constFromIntVar = constFromVar->getValue().getSExtValue();
            curApDomain.performUnaryOp(STORE, destVarName, constFromIntVar);
        }
        else if (Argument *argFromVar = dyn_cast<Argument>(fromVar)) {
            // TODO: handle function arguments

        }
        else {
            string fromVarName = getNameFromValue(fromVar);
            curApDomain.performUnaryOp(STORE, destVarName, fromVarName);
        }

        return curApDomain;
    }

    ApDomain checkUnaryInst(
        UnaryInstruction* unaryInst, 
        ApDomain curApDomain, 
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
                    curApDomain = applyInterfToLoad(unaryInst, curApDomain, interf, fromVarName);
                }
                break;
            // TODO: add more cases
            default: 
                fprintf(stderr, "ERROR: unknown operation: ");
                unaryInst->print(errs());
                return curApDomain;
        }
        
        curApDomain.performUnaryOp(oper, destVarName, fromVarName);

        return curApDomain;
    }

    ApDomain applyInterfToLoad(
        UnaryInstruction* unaryInst, 
        ApDomain curApDomain, 
        unordered_map<Instruction*, Instruction*> interf,
        string varName
    ) {
        errs() << "Applying interf\n";
        // find interfering instruction
        auto searchInterf = interf.find(unaryInst);
        if (searchInterf == interf.end()) {
            errs() << "ERROR: Interfernce for the load instrction not found\n";
            printValue(unaryInst);
            return curApDomain;
        }
        Instruction *interfInst = searchInterf->second;
        
        // if interfernce is from some other thread
        if (interfInst != unaryInst->getPrevNode()) {
            // find the domain of interfering instruction
            auto searchInterfFunc = programState.find(interfInst->getFunction());
            if (searchInterfFunc != programState.end()) {
                auto searchInterfApDomain = searchInterfFunc->second.find(interfInst);
                // errs() << "For Load: ";
                // unaryInst->print(errs());
                // errs() << "\nInterf with Store: ";
                // interfInst->print(errs());
                // errs() << "\n";
                if (searchInterfApDomain != searchInterfFunc->second.end()) {
                    // apply the interference
                    // errs() << "Before Interf:\n";
                    // curApDomain.printApDomain();

                    // TODO: need to set the bool value proplerly
                    ApDomain interfApDomain = searchInterfApDomain->second;
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
                                //     curApDomain.setRelHead(varName, relHead);
                                //     isRelSeq = true;
                                // }
                            }
                        }
                    }
                    curApDomain.applyInterference(varName, interfApDomain, isRelSeq);
                    
                    // errs() << "***After Inter:\n";
                    // curApDomain.printApDomain();
                }
            }
        }
        return curApDomain;
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
        unordered_map<Function*, unordered_map<string, unordered_set<Instruction*>>> allStores
    ){
        unordered_map<Function*, vector< unordered_map<Instruction*, Instruction*>>> allInterfs;
        unordered_map<Function*, unordered_map<Instruction*, vector<Instruction*>>> loadsToAllStores;
        // Make all permutations
        // TODO: add dummy env i.e. load from itself
        loadsToAllStores = getLoadsToAllStoresMap(allLoads, allStores);
        allInterfs = getAllInterferences(loadsToAllStores);

        // TODO: Check feasibility of permutations and save them in feasibleInterfences
        feasibleInterfences = allInterfs;
        // printFeasibleInterf();

        return allInterfs;
    }

    unordered_map<Instruction*, ApDomain> joinApDomainByInstruction(
        unordered_map<Instruction*, ApDomain> instrToApDomainOld,
        unordered_map<Instruction*, ApDomain> instrToApDomainNew
    ) {
        // errs() << "joining domains. Incoming ApDomain:\n";
        // errs() << "siez of OLD= " << instrToApDomainOld.size() << "\n";
        // errs() << "size of NEW= " << instrToApDomainNew.size() << "\n";

        
        // new = old join new
        for (auto itOld=instrToApDomainOld.begin(); itOld!=instrToApDomainOld.end(); ++itOld) {
            // errs() << "joining for instruction: ";
            // itOld->first->print(errs());
            // errs() << "\n";
            auto searchNewMap = instrToApDomainNew.find(itOld->first);
            if (searchNewMap == instrToApDomainNew.end()) {
                instrToApDomainNew[itOld->first] = itOld->second;
            } else {
                ApDomain newApDomain = searchNewMap->second;
                // errs() << "OLD:\n";
                // itOld->second.printApDomain();
                // errs() << "NEW:\n";
                // newApDomain.printApDomain();
                newApDomain.joinApDomain(itOld->second);
                // errs() << "Joined domain:\n";
                // newApDomain.printApDomain();
                instrToApDomainNew[itOld->first] = newApDomain;
            }
        }
        // errs() << "***In join domain. Before return:\n";
        // printInstToApDomainMap(instrToApDomainNew);

        return instrToApDomainNew;
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
        auto funIt = funcInitApDomain.begin();
        Function *fun1 = funIt->first;
        ApDomain fun1ApDomain = funIt->second;
        fun1ApDomain.performUnaryOp(STORE, "x", 1);
        fun1ApDomain.performUnaryOp(STORE, "y", 10);
        funIt++;
        Function *fun2 = funIt->first;
        ApDomain fun2ApDomain = funIt->second;
        errs() << "Interf from domain:\n";
        fun1ApDomain.printApDomain();
        errs() << "To domain:\n";
        fun2ApDomain.printApDomain();
        fun2ApDomain.applyInterference("x", fun1ApDomain, true);
        errs() << "After applying:\n";
        fun2ApDomain.printApDomain();
    }

    void printInstToApDomainMap(unordered_map<Instruction*, ApDomain> instToApDomainMap) {
        for (auto it=instToApDomainMap.begin(); it!=instToApDomainMap.end(); ++it) {
            it->first->print(errs());
            it->second.printApDomain();
        }
    }

    void printProgramState() {
        for (auto it1=programState.begin(); it1!=programState.end(); ++it1) {
            errs() << "\n-----------------------------------------------\n";
            errs() << "Function " << it1->first->getName() << ":\n";
            errs() << "-----------------------------------------------\n";
            printInstToApDomainMap(it1->second);
        }
    }

    bool isFixedPoint(unordered_map <Function*, unordered_map<Instruction*, ApDomain>> newProgramState) {
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
                    it3->second->print(errs());
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

    Function* findFunctionFromJoin(Instruction* call) {
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


    vector<Instruction*> getLastInstOfJoin(Instruction *call) {
        vector<Instruction*> lastInstr;
        Function *func = findFunctionFromJoin(call);
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
