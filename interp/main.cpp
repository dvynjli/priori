#include "common.h"

class IntVar {
    string name;
    int val;        //TODO: change to value in abstarct domain and init accordingly

    public:
        IntVar(string name, int val) : name(name), val(val) {}
};

struct VerifierPass : public ModulePass {
    static char ID;
    VerifierPass() : ModulePass(ID) {}

    bool runOnModule (Module &M) {
        errs() << "LLVM pass is running\n";
        
        ap_manager_t *ap_man = box_manager_alloc();                     //TODO: depend upon command line arg
        
    }

    ap_environment_t * initEnvironment(Module &M) {
        vector<IntVar> intVars;
        for (auto it = M.global_begin(); it != M.global_end(); it++){
            print_msg(2, "Global var: ", it->getName());
            if (it->getType()->getElementType()->isIntegerTy()) {
                intVars.push_back(IntVar(it->getName(), 0));
            }
        }
        print_debug_msg(2, "Total global var = ", to_string(intVars.size()).c_str());
        ap_var_t ints[intVars.size()];
        
        return nullptr;
        //return ap_environment_alloc(intArr, intArr.size(), f)
    }
};

char VerifierPass::ID = 0;
static RegisterPass<VerifierPass> X("verifier", "Abstarct Interpretation Verifier Pass", false, true /*change it to true for analysis pass*/);
