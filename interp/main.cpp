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
    unordered_map <Function*, unordered_map<Instruction*, Domain>> programState;
    unordered_map <Function*, Domain> funcInitDomain;
    unordered_map <Function*, vector< unordered_map<Instruction*, Instruction*>>> feasibleInterfences;
    unordered_map <string, Value*> nameToValue;
    unordered_map <Value*, string> valueToName;
    Z3Helper zHelper;

    
    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        // Domain initDomain = Domain();
        // TODO: get domain type based on comman line arguments
        string domainType = "box";
        
        // zHelper.testFixedPoint();

        vector<string> globalVars = getGlobalIntVars(M);
        initThreadDetails(M, globalVars, domainType);

        // zHelper.initZ3(globalVars);

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
            vector<string> funcVars(globalVars);
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
                                    // Instruction *lastGlobalInstBeforeCall = getLastGlobalInst(call);
                                    // Instruction *firstGlobalInstInCalled  = getNextGlobalInst(&*(newThread->begin()->begin()));
                                    // // lastGlobalInstBeforeCall (or firstGlobalInstInCalled) == nullptr means there 
                                    // // no global instr before thread create in current function (or in newly created thread)
                                    // if (lastGlobalInstBeforeCall) {
                                    //     zHelper.addMHB(lastGlobalInstBeforeCall, call);
                                    // }
                                    // if (firstGlobalInstInCalled) {
                                    //     zHelper.addMHB(call, firstGlobalInstInCalled);
                                    // }
                                }

                            }
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
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
                            // zHelper.addStoreInstr(storeInst);
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
                                // loadInst->print(errs());
                                // errs() << "\n";
                                // zHelper.addLoadInstr(loadInst);
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

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs
        unsigned int iterations = 0;
        unordered_map <Function*, unordered_map<Instruction*, Domain>> programStateCurItr;
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
                unordered_map<Instruction*, Domain> newFuncDomain;

                auto searchInterf = feasibleInterfences.find(curFunc);
                
                if (searchInterf != feasibleInterfences.end()) {
                    curFuncInterfs = (searchInterf->second);
                } else {
                    errs() << "WARNING: No interf found for Function. It will be analyzed only ones.\n";
                    if (iterations == 0) {
                        unordered_map <Instruction*, Instruction*> interf;
                        newFuncDomain = analyzeThread(*funcItr, interf);
                        programStateCurItr.emplace(curFunc, newFuncDomain);
                    }
                }
                
                // errs() << "Number of interf= " << curFuncInterfs.size();
                // analyze the Thread for each interference
                for (auto interfItr=curFuncInterfs.begin(); interfItr!=curFuncInterfs.end(); ++interfItr){
                    // errs() << "\n***For new interf\n";

                    newFuncDomain = analyzeThread(*funcItr, *interfItr);

                    // errs() << "Domain after analysis:\n";
                    // printInstToDomainMap(newFuncDomain);

                    // join newFuncDomain of all feasibleInterfs and replace old one in state
                    auto searchFunDomain = programStateCurItr.find(curFunc);
                    if (searchFunDomain == programStateCurItr.end()) {
                        // errs() << "curfunc not found in program state\n";
                        programStateCurItr.emplace(curFunc, newFuncDomain);
                    }
                    else {
                        // errs() << "curfunc already exist in program state. joining\n";
                        programStateCurItr[curFunc] =  joinDomainByInstruction(searchFunDomain->second, newFuncDomain);
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

    unordered_map<Instruction*, Domain> analyzeThread (Function *F, unordered_map<Instruction*, Instruction*> interf) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        unordered_map <Instruction*, Domain> curFuncDomain;

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

            predDomain = analyzeBasicBlock(currentBB, predDomain, curFuncDomain, interf);
        }

        // errs() << "\nDomain of function:\n";
        // printInstToDomainMap(curFuncDomain);

        return curFuncDomain;
    }

    Domain analyzeBasicBlock (BasicBlock *B, 
        Domain predDomain, 
        unordered_map <Instruction*, Domain> &curFuncDomain,
        unordered_map<Instruction*, Instruction*> interf
    ) {
        // check type of inst, and performTrasformations
        Domain curDomain;
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);
            curDomain.copyDomain(predDomain);
        
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
                curDomain = checkUnaryInst(unaryInst, curDomain, interf);
            }
            else if (CmpInst *cmpInst = dyn_cast<CmpInst> (instItr)) {
                errs() << "cmpInst: ";
                cmpInst->print(errs());
                errs() << "\n";
                curDomain = checkCmpInst(cmpInst, curDomain);
                
            }
            else {
                
            }
            // RMW, CMPXCHG

            curFuncDomain[currentInst] = curDomain;
            predDomain.copyDomain(curDomain);
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

    Domain checkCmpInst(CmpInst* cmpInst, Domain curDomain) { 
        // need to computer Domain of both true and false branch
        operation operTrueBranch;
        switch (cmpInst->getPredicate()) {
            case CmpInst::Predicate::ICMP_EQ:
                operTrueBranch = EQ;
                break;
            case CmpInst::Predicate::ICMP_NE:
                operTrueBranch = NE;
                break;
            case CmpInst::Predicate::ICMP_SGT:
                operTrueBranch = GT;
                break;
            case CmpInst::Predicate::ICMP_SGE:
                operTrueBranch = GE;
                break;
            case CmpInst::Predicate::ICMP_SLT:
                operTrueBranch = LT;
                break;
            case CmpInst::Predicate::ICMP_SLE:
                operTrueBranch = LE;
                break;
            default:
                errs() << "WARNING: Unknown cmp instruction: ";
                cmpInst->print(errs());
                errs() << "\n";
                return curDomain;
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
                curDomain.performCmpOp(operTrueBranch, destVarName, constFromIntVar1, constFromIntVar2);
            }
            else { 
                string fromVar2Name = getNameFromValue(fromVar2);
                curDomain.performCmpOp(operTrueBranch, destVarName, constFromIntVar1, fromVar2Name);
            }
        }
        else if (ConstantInt *constFromVar2 = dyn_cast<ConstantInt>(fromVar2)) {
            string fromVar1Name = getNameFromValue(fromVar1);
            int constFromIntVar2 = constFromVar2->getValue().getSExtValue();
            curDomain.performCmpOp(operTrueBranch, destVarName, fromVar1Name, constFromIntVar2);
        }
        else {
            string fromVar1Name = getNameFromValue(fromVar1);
            string fromVar2Name = getNameFromValue(fromVar2);
            curDomain.performCmpOp(operTrueBranch, destVarName, fromVar1Name, fromVar2Name);
        }

        return curDomain;
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

    Domain checkUnaryInst(
        UnaryInstruction* unaryInst, 
        Domain curDomain, 
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
                    curDomain = applyInterfToLoad(unaryInst, curDomain, interf, fromVarName);
                }
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

    Domain applyInterfToLoad(
        UnaryInstruction* unaryInst, 
        Domain curDomain, 
        unordered_map<Instruction*, Instruction*> interf,
        string varName
    ) {
        errs() << "Applying interf\n";
        // find interfering instruction
        auto searchInterf = interf.find(unaryInst);
        if (searchInterf == interf.end()) {
            errs() << "ERROR: Interfernce for the load instrction not found\n";
            unaryInst->print(errs());
            errs() << "\n";
            return curDomain;
        }
        Instruction *interfInst = searchInterf->second;
        
        // if interfernce is from some other thread
        if (interfInst != unaryInst->getPrevNode()) {
            // find the domain of interfering instruction
            auto searchInterfFunc = programState.find(interfInst->getFunction());
            if (searchInterfFunc != programState.end()) {
                auto searchInterfDomain = searchInterfFunc->second.find(interfInst);
                // errs() << "For Load: ";
                // unaryInst->print(errs());
                // errs() << "\nInterf with Store: ";
                // interfInst->print(errs());
                // errs() << "\n";
                if (searchInterfDomain != searchInterfFunc->second.end()) {
                    // apply the interference
                    // errs() << "Before Interf:\n";
                    // curDomain.printDomain();

                    curDomain.applyInterference(varName, searchInterfDomain->second);
                    
                    // errs() << "***After Inter:\n";
                    // curDomain.printDomain();
                }
            }
        }
        return curDomain;
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

    unordered_map<Instruction*, Domain> joinDomainByInstruction(
        unordered_map<Instruction*, Domain> instrToDomainOld,
        unordered_map<Instruction*, Domain> instrToDomainNew
    ) {
        // errs() << "joining domains. Incoming Domain:\n";
        // errs() << "siez of OLD= " << instrToDomainOld.size() << "\n";
        // errs() << "size of NEW= " << instrToDomainNew.size() << "\n";

        
        // new = old join new
        for (auto itOld=instrToDomainOld.begin(); itOld!=instrToDomainOld.end(); ++itOld) {
            // errs() << "joining for instruction: ";
            // itOld->first->print(errs());
            // errs() << "\n";
            auto searchNewMap = instrToDomainNew.find(itOld->first);
            if (searchNewMap == instrToDomainNew.end()) {
                instrToDomainNew[itOld->first] = itOld->second;
            } else {
                Domain newDomain = searchNewMap->second;
                // errs() << "OLD:\n";
                // itOld->second.printDomain();
                // errs() << "NEW:\n";
                // newDomain.printDomain();
                newDomain.joinDomain(itOld->second);
                // errs() << "Joined domain:\n";
                // newDomain.printDomain();
                instrToDomainNew[itOld->first] = newDomain;
            }
        }
        // errs() << "***In join domain. Before return:\n";
        // printInstToDomainMap(instrToDomainNew);

        return instrToDomainNew;
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
        auto funIt = funcInitDomain.begin();
        Function *fun1 = funIt->first;
        Domain fun1Domain = funIt->second;
        fun1Domain.performUnaryOp(STORE, "x", 1);
        funIt++;
        Function *fun2 = funIt->first;
        Domain fun2Domain = funIt->second;
        fun2Domain.applyInterference("x", fun1Domain);
    }

    void printInstToDomainMap(unordered_map<Instruction*, Domain> instToDomainMap) {
        for (auto it=instToDomainMap.begin(); it!=instToDomainMap.end(); ++it) {
            it->first->print(errs());
            it->second.printDomain();
        }
    }

    void printProgramState() {
        for (auto it1=programState.begin(); it1!=programState.end(); ++it1) {
            errs() << "\n-----------------------------------------------\n";
            errs() << "Function " << it1->first->getName() << ":\n";
            errs() << "-----------------------------------------------\n";
            printInstToDomainMap(it1->second);
        }
    }

    bool isFixedPoint(unordered_map <Function*, unordered_map<Instruction*, Domain>> newProgramState) {
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

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true);
