#ifndef __INTERF__
#define __INTERF__

#include "common.h"

class ThreadNodeToEnv {
    //Tuple <n,e>, where n is node in the program graph of thread and e is environment 
    map<Value*, ap_abstract1_t> TE;
    public:
        ThreadNodeToEnv() {}
        // ThreadNodeToEnv(Value* n, ap_abstract1_t e)  {
        //     TE.insert(pair<Value*, ap_abstract1_t> (n, e));
        // }
       
        //Join the current enviornment with other on matching nodes
         ap_abstract1_t insertOrJoinOnNode(ap_manager_t *man, Value* node, ap_abstract1_t env){
            auto result = TE.find(node);
            if (result != TE.end()){
                ap_abstract1_t new_env = ap_abstract1_join(man, false, &(result->second), &env);
                return new_env;
            }
            else {
                TE.insert(pair<Value*, ap_abstract1_t> (node, env));
                return env;
            }
        }

        ap_abstract1_t getEnv(Value* node) {
            auto result = TE.find(node);
            if (result != TE.end())
                return result->second;
            else
                return ;
        }  
};

class Intrefernce {
    //Interfernces from each thread. 
    Function *thread;
    ThreadNodeToEnv interfs;

    public: 
        Intrefernce() {}
        Intrefernce(Function* thread, ThreadNodeToEnv interfs) : thread(thread), interfs(interfs) {}

        void insertOrJoinInterference(ap_manager_t* man, Value* node, ap_abstract1_t env) {
            interfs.insertOrJoinOnNode(man, node, env);
        }


};

#endif
