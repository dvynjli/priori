#include "common.h"

class IntVar {
    string name;
    int val;        //TODO: change to value in abstarct domain and init accordingly

    public:
        IntVar(string name, int val) : name(name), val(val) {}
};

class VerifierPass : public ModulePass {
    /* TODO: Should I create threads as a set??
        Vector- interferences of t1 with t1 will be explored in case more than one thread of same func are present
        Set and interference of each thread with itself are also explored, support to inf threads of same func can be added 
    */
    vector<Function*> threads;

    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        
        ap_manager_t *man = initApManager();
        fprintf(stderr, "Init Env\n");
        ap_environment_t *env = initEnvironment(M);
        ap_environment_fdump(stdout, env);
         //ap_var_t var = ap_environment_var_of_dim(env, 0);
        fprintf(stderr, "creating top\n");
        ap_abstract1_t top = ap_abstract1_top(man, env);
        ap_abstract1_fdump(stderr, man,  &top);

        initThreadDetails(M);
        
    }

    ap_environment_t * initEnvironment(Module &M) {
        vector<string> intVars;
        for (auto it = M.global_begin(); it != M.global_end(); it++){
            fprintf(stderr, "Global var: %s\n", it->getName());
            if (it->getType()->getElementType()->isIntegerTy()) {
                intVars.push_back(it->getName());
            }
        }
        fprintf(stderr, "DEBUG: Total global var = %d\n", intVars.size());
        ap_var_t intAp[intVars.size()];
        int i = 0;
        for (auto it = intVars.begin(); it != intVars.end(); it++, i++){
            intAp[i] = (ap_var_t)(it->c_str());
        }
        ap_var_t floatAp[0];
        
        //return nullptr;
        return ap_environment_alloc(intAp, intVars.size(), floatAp, 0);
    }

    ap_manager_t* initApManager()
    {
        //TODO: depend upon command line arg
        return box_manager_alloc();
    }

    void initThreadDetails(Module &M) {
        //find main function
        Function *mainF;
        for(auto funcItr = M.begin(); funcItr != M.end(); funcItr++) {
            if (funcItr->getName() == "main"){
                mainF = (Function*)funcItr;
                break;    
            }
        }

        threads.push_back(mainF);
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
                        }
                    }
                }
            }
        }
    }

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true /*change it to true for analysis pass*/);
