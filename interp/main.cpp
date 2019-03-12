#include "common.h"
#include "domain.h"
#include "analyzer.h"
#include "z3_handler.h"

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
    ProgramState program_state;

    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        program_state = ProgramState();
        // TODO: get domain type based on comman line arguments
        string domain_type = "box";
        program_state.init(domain_type, getGlobalIntVars(M));
        
        initThreadDetails(M);

        analyzeProgram(M);

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
                mainF = (Function*)funcItr;
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

    void analyzeProgram(Module &M) {
        // call analyzThread, get interf, check fix point
        // need to addRule, check feasible interfs

    }

    void analyzeThread(Function &F) {
        //call analyze BB, do the merging of BB
    }

    void analyzeBasicBlock(BasicBlock &B) {
        // check type in inst, and performTrasformations
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
