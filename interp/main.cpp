#include "common.h"
#include "domain.h"
#include "analyzer.h"
#include "z3_handler.h"
#include "llvm/IR/CFG.h"

class IntVar {
    string name;
    int val;        //TODO: change to value in abstarct domain and init accordingly

    public:
        IntVar(string name, int val) : name(name), val(val) {}
};

class VerifierPass : public ModulePass {

    /* TODO: Should I create threads as a set or vector??
        Vector- interferences of t1 with t1 will be explored in case more than one thread of same func are present
        Set and interference of each thread with itself are also explored, support to inf threads of same func can be added 
    */
    vector<Function*> threads;
    map<Function*, map<Instruction*, Domain>> programState;
    map <string, Value*> nameToValue;
    map <Value*, string> valueToName;

    
    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        Domain initDomain = Domain();
        // TODO: get domain type based on comman line arguments
        string domainType = "box";
        initDomain.init(domainType, getGlobalIntVars(M));
        
        initThreadDetails(M);

        analyzeProgram(M, initDomain);

        // unsat_core_example1();
    }

    vector<string> getGlobalIntVars(Module &M) {
        vector<string> intVars;
        for (auto it = M.global_begin(); it != M.global_end(); it++){
            // cerr << "Global var: " << it->getName() << endl;
            fprintf(stderr, "Global var: %s of type: %d\n", it->getName(), it->getValueType()->getTypeID());
            if (it->getValueType()->isIntegerTy()) {
                intVars.push_back(it->getName());
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

    void initThreadDetails(Module &M) {
        //find main function
        Function *mainF = getMainFunction(M);

        threads.push_back(mainF);

        // TODO: Why queue? Is it better to take it as set
        queue<Function*> funcQ;
        funcQ.push(mainF);
        // for(auto tIt= threads.begin(); tIt != threads.end(); tIt++)                 //all threads created so far

        int localVarCounter = 0;

        while(!funcQ.empty())
        {
            Function *func = funcQ.front();
            funcQ.pop();
            for(auto block = func->begin(); block != func->end(); block++)          //iterator of Function class over BasicBlock
            {
                for(auto it = block->begin(); it != block->end(); it++)       //iterator of BasicBlock over Instruction
                {
                    if (CallInst *call = dyn_cast<CallInst>(it)) {
                        if(!call->getCalledFunction()->getName().compare("pthread_create")) {
                            if (Function* newThread = dyn_cast<Function> (call->getArgOperand(2)))
                            {  
                                threads.push_back(newThread);
                                funcQ.push(newThread);
                            }
                            // TODO: need to add dominates rules
                        }
                        else if (!call->getCalledFunction()->getName().compare("pthread_join")) {
                            // TODO: need to add dominates rules
                        }
                        else {
                            cout << "unknown function call:\n";
                            it->dump();
                        }
                    }
                    else if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(it)) {
                        string varName = "var" + to_string(localVarCounter);
                        localVarCounter++;
                        fprintf(stderr, "DEBUG: found local var named %s\n", varName.c_str());
                        nameToValue.emplace(varName, allocaInst);
                        valueToName.emplace(allocaInst, varName);
                        // domain.add_variable(varName);
                    }
                    /*
                    else
                        checkInstruction(&(*it));
                      if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(it)) {
                        if (binOp->getOpcode() == Instruction::Add) {
                            // fprintf(stderr, "Found add inst: ");
                            // binOp->dump();

                        }
                        else{
                            // fprintf(stderr, "Op found: %s\n", binOp->getOpcodeName());
                        }
                    }
                    else {
                        // fprintf(stderr, "Opcode of instruction: %d %s\n", it->getOpcode(), it->getOpcodeName());
                    } */
                }
            }
        }
    }

    void analyzeProgram(Module &M, Domain initDomain) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs

        map<Function*, map<Instruction*, Instruction*>> feasibleInterf;
        
        map<Instruction*, Instruction*> *curFuncInterf;
        for (auto funcItr=threads.begin(); funcItr!=threads.end(); ++funcItr){
            fprintf(stderr, "DEBUG: Analyzing thread %s\n", (*funcItr)->getName());

            // find feasible interfernce for current function
            auto searchInterf = feasibleInterf.find(*funcItr);
            if (searchInterf != feasibleInterf.end()) {
                curFuncInterf = &(searchInterf->second);
            }

            // map<Instruction*, Domain> *cur_func_domain;
            // // find domain of current function
            // auto searchDomain = programState.find(*funcItr);
            // if (programState.find(*funcItr) != programState.end()) {
            //     cur_func_domain = &(searchDomain->second);
            // }
            
            // TODO: Program state of each function might have different local varibales.
            
            map<Instruction*, Domain> newFuncInterf;
            newFuncInterf = analyzeThread(*funcItr, curFuncInterf, initDomain);
            // join newFuncInterf of all feasibleInterfs and replace old one

            // curFuncInterf->clear();
        }
    }

    map<Instruction*, Domain> analyzeThread(Function *F, map<Instruction*, Instruction*> *interf, Domain initDomain) {
        //call analyze BB, do the merging of BB depending upon term condition
        //init for next BB with assume

        map <Instruction*, Domain> curFuncDomain;
        // Domain predDomain;

        for(auto bbItr=F->begin(); bbItr!=F->end(); ++bbItr){
            BasicBlock *currentBB = &(*bbItr);

            Domain predDomain = initDomain;
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

    Domain analyzeBasicBlock(BasicBlock *B, Domain curDomain, map <Instruction*, Domain> curFuncDomain) {
        // check type of inst, and performTrasformations
        
        for (auto instItr=B->begin(); instItr!=B->end(); ++instItr) {
            Instruction *currentInst = &(*instItr);

            if (AllocaInst *allocaInst = dyn_cast<AllocaInst>(currentInst)) {
                auto searchName = valueToName.find(currentInst);
                if (searchName == valueToName.end()) {
                    fprintf(stderr, "ERROR: new mem alloca");
                    exit(0);
                }
                curDomain.addVariable(searchName->second);
            }

        }
        
        return curDomain;
    }

    //  call approprproate function for the inst passed
    void checkInstruction(Instruction* inst){

    }

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true /*change it to true for analysis pass*/);
