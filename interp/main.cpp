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
    map <string, Value*> nameToValue;
    map <Value*, string> valueToName;

    
    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        Domain initDomain = Domain();
        // TODO: get domain type based on comman line arguments
        string domainType = "box";
        initDomain.init(domainType, getGlobalIntVars(M));
        
        initThreadDetails(M, initDomain);

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

    void initThreadDetails(Module &M, Domain initDomain) {
        //find main function
        Function *mainF = getMainFunction(M);

        threads.push_back(mainF);

        // TODO: Why queue? Is it better to take it as set
        queue<Function*> funcQ;
        unordered_set<Function*> funcSet;
        funcQ.push(mainF);
        funcSet.insert(mainF);
        // for(auto tIt= threads.begin(); tIt != threads.end(); tIt++)                 //all threads created so far

        int ssaVarCounter = 0;

        while(!funcQ.empty())
        {
            Function *func = funcQ.front();
            funcQ.pop();
            Domain curFuncDomain(initDomain);
            for(auto block = func->begin(); block != func->end(); block++)          //iterator of Function class over BasicBlock
            {
                for(auto it = block->begin(); it != block->end(); it++)       //iterator of BasicBlock over Instruction
                {
                    if (CallInst *call = dyn_cast<CallInst>(it)) {
                        if(!call->getCalledFunction()->getName().compare("pthread_create")) {
                            if (Function* newThread = dyn_cast<Function> (call->getArgOperand(2)))
                            {  
                                threads.push_back(newThread); 	
                                auto inSet = funcSet.insert(newThread);
                                if (inSet.second)
                                    funcQ.push(newThread);
                            }
                            // TODO: need to add dominates rules
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
                        }
                        else {
                            cout << "unknown function call:\n";
                            // it->dump();
                            it->print(errs());
                        }
                    }
                    else if (StoreInst *storeInst = dyn_cast<StoreInst>(it)) {

                    }
                    else if (it->isTerminator()) {

                    }

                    else {
                        Instruction *inst = dyn_cast<Instruction>(it);
                        string varName = "var" + to_string(ssaVarCounter);
                        ssaVarCounter++;
                        nameToValue.emplace(varName, inst);
                        valueToName.emplace(inst, varName);
                        curFuncDomain.addVariable(varName);
                    }
                }
            }
            funcInitDomain[func] = curFuncDomain;
        }
    }

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs

        map<Function*, map<Instruction*, Instruction*>> feasibleInterf;
        
        map<Instruction*, Instruction*> *curFuncInterf;
        for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
            fprintf(stderr, "\n******DEBUG: Analyzing thread %s*****\n", (*funcItr)->getName());

            // find feasible interfernce for current function
            auto searchInterf = feasibleInterf.find(*funcItr);
            if (searchInterf != feasibleInterf.end()) {
                curFuncInterf = &(searchInterf->second);
            }
            
            map<Instruction*, Domain> newFuncDomain;
            map<string, vector<Instruction*>> *varToStores;
            newFuncDomain = analyzeThread(*funcItr, curFuncInterf, varToStores);
            // join newFuncDomain of all feasibleInterfs and replace old one in state

            // curFuncInterf->clear();
        }
    }

    map<Instruction*, Domain> analyzeThread(
                Function *F, 
                map<Instruction*, Instruction*> *interf, 
                map<string, vector<Instruction*>> *varToStores) {
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

            predDomain = analyzeBasicBlock(currentBB, predDomain, curFuncDomain, varToStores);
        }
        return curFuncDomain;
    }

    Domain analyzeBasicBlock(
        BasicBlock *B, 
        Domain curDomain, 
        map <Instruction*, Domain> curFuncDomain, 
        map<string, vector<Instruction*>> *varToStores) {
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
                binOp->dump();
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
        operation oper;
        switch (unaryInst->getOpcode()) {
            case Instruction::Load:
                oper = LOAD;
                break;
            // TODO: add more cases
            default: 
                fprintf(stderr, "ERROR: unknown operation: ");
                unaryInst->dump();
                return curDomain;
        }
        Value* fromVar = unaryInst->getOperand(0);
        string fromVarName = getNameFromValue(fromVar);
        string destVarName = getNameFromValue(unaryInst);

        curDomain.performUnaryOp(oper, destVarName, fromVarName);

        return curDomain;
    }

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true /*change it to true for analysis pass*/);
