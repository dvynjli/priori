#include "common.h"

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

    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        
        ap_manager_t *man = initApManager();
        fprintf(stderr, "Init Env\n");
        ap_environment_t *env = initEnvironment(M);
        ap_environment_fdump(stderr, env);
        // ap_var_t var = ap_environment_var_of_dim(env, 0);
        // fprintf(stderr, "var: %s\n", var);
        fprintf(stderr, "creating top\n");
        ap_abstract1_t abs_val = ap_abstract1_top(man, env);
        // ap_abstract1_fdump(stderr, man,  &top);
        
        // initThreadDetails(M);
        performTrasfer(man, env, abs_val);
        ap_abstract1_fdump(stderr, man,  &abs_val);
    }

    void performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t abs_val) {
        /* assign x = 1 */
        fprintf(stderr, "Assigning x = 1\n");
        ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
        ap_linexpr1_set_list(&expr, AP_CST_S_INT, 1, AP_END);
        ap_linexpr1_fprint(stderr, &expr);
        ap_var_t var = ap_environment_var_of_dim(env, 0);
        abs_val = ap_abstract1_assign_linexpr(man, true, &abs_val, var, &expr, NULL);
        fprintf(stderr, "assigned linexpr to x\n");
        ap_abstract1_fprint(stderr, man, &abs_val);
        // ap_linexpr1_clear(&expr);

        /* assign y = x + 10 */
        fprintf(stderr, "Assigning y = x + 10\n");
        ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, "x", AP_CST_S_INT, 10, AP_END);
        ap_linexpr1_fprint(stderr, &expr);
        var = ap_environment_var_of_dim(env, 1);
        abs_val = ap_abstract1_assign_linexpr(man, true, &abs_val, var, &expr, NULL);
        fprintf(stderr, "assigned linexpr to x\n");
        ap_abstract1_fprint(stderr, man, &abs_val);
        // ap_linexpr1_clear(&expr);

        /* assign x = x + y */
        fprintf(stderr, "Assigning x = x + y\n");
        ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, "x", AP_COEFF_S_INT, 1, "y", AP_CST_S_INT, 0, AP_END);
        ap_linexpr1_fprint(stderr, &expr);
        var = ap_environment_var_of_dim(env, 0);
        abs_val = ap_abstract1_assign_linexpr(man, true, &abs_val, var, &expr, NULL);
        fprintf(stderr, "assigned linexpr to x\n");
        ap_abstract1_fprint(stderr, man, &abs_val);
        
        ap_linexpr1_clear(&expr);

    }

    ap_environment_t * initEnvironment(Module &M) {
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
        ap_var_t intAp[intVars.size()];
        int i = 0;
        for (auto it = intVars.begin(); it != intVars.end(); it++, i++){
            intAp[i] = (ap_var_t)(it->c_str());
        }
        //TODO: use if providing support for floats
        ap_var_t floatAp[0];
        
        //return nullptr;
        return ap_environment_alloc(intAp, intVars.size(), floatAp, 0);
    }

    ap_manager_t* initApManager()
    {
        //TODO: parameterize by command line arg
        return box_manager_alloc();
    }

    //  call approprproate function for the inst passed
    void checkInstruction(Instruction* inst){

    }

    void initThreadDetails(Module &M) {
        //find main function
        Function *mainF;

        //TODO: llvm should have a better way of finding main function
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
                        //TODO: what should be done for non-pthread create function call
                    }
                    else
                        checkInstruction(&(*it));
                    /*  if (BinaryOperator *binOp = dyn_cast<BinaryOperator>(it)) {
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

    public:
        static char ID;
        VerifierPass() : ModulePass(ID) {}
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true /*change it to true for analysis pass*/);
