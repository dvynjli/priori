#ifndef __DOMAIN
#define __DOMAIN

#include "common.h"

#include "ap_abstract1.h"
#include "ap_manager.h"
#include "box.h"
#include "ap_global1.h"

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

class Domain {
    ap_manager_t *man;
    ap_environment_t *env;
    ap_abstract1_t absValue;
    map<string, llvm::Instruction*> relHead;
    map<string, bool> hasChanged;
    
    ap_manager_t* initApManager(string domainType);
    ap_environment_t* initEnvironment(vector<string> globalVars, vector<string> functionVars);
    void assignZerosToAllVars();
    void initRelHead(vector<string> globalVars);
    void initHasChanged(vector<string> globalVars);
    ap_constyp_t getApConsType(operation oper);
    void setHasChanged(string var);
    
    void performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t abs_val);

public:
    bool operator== (const Domain &other) const;
    // bool operator!= (Domain other);
    void init(string domainType, vector<string> globalVars, vector<string> functionVars);
    void copyDomain(Domain copyFrom);
    llvm::Instruction* getRelHead(string var);
    void setRelHead(string var, llvm::Instruction *head);

    // Unary Operations
    void performUnaryOp(operation oper, string strTo, string strOp);
    void performUnaryOp(operation oper, string strTo, int intOp);
    // Binary Operations
    void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2);
    void performBinaryOp(operation oper, string strTo, int intOp1,    string strOp2);
    void performBinaryOp(operation oper, string strTo, int intOp1,    int intOp2);
    void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2);
    // Other Operations
    void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);
    // Cmp Operations
    void performCmpOp(operation oper, string strOp1, int intOp2);
    void performCmpOp(operation oper, int intOp1,    string strOp2);
    void performCmpOp(operation oper, int intOp1,    int intOp2);
    void performCmpOp(operation oper, string strOp1, string strOp2);
    
    void applyInterference(string interfVar, Domain fromDomain, bool isRelAcqSeq);
    bool joinDomain(Domain other);

    void addVariable(string varName);
    void printDomain();
};


#endif