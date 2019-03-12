#ifndef __DOMAIN
#define __DOMAIN

#include "common.h"

#include "ap_abstract1.h"
#include "ap_manager.h"
#include "box.h"
#include "ap_global1.h"

class Domain {
    ap_manager_t *man;
    ap_environment_t *env;
    ap_abstract1_t absValue;
    ap_manager_t* initApManager(string domainType);
    ap_environment_t* initEnvironment(vector<string> intVars);
    
    void performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t abs_val);

    public:
    void init(string domainType, vector<string> intVars);
    void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2);
    void performUnaryOp(operation oper, string strTo, string strOp);
    void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);
    void joinDomain(Domain other);
    void addVariable(string varName);
};


#endif