#ifndef __DOMAIN
#define __DOMAIN

#include "common.h"

#include "ap_abstract1.h"
#include "ap_manager.h"
#include "box.h"
#include "oct.h"
#include "ap_global1.h"

#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

typedef map <string, llvm::Instruction*> REL_HEAD;
extern llvm::cl::opt<DomainTypes> AbsDomType;

class ApDomain {
    ap_manager_t *man;
    ap_environment_t *env;
    ap_abstract1_t absValue;
    // map<string, llvm::Instruction*> relHead;
    map<string, bool> hasChanged;
    
    ap_manager_t* initApManager();
    ap_environment_t* initEnvironment(vector<string> globalVars, vector<string> functionVars);
    void assignZerosToAllVars();
    void initHasChanged(vector<string> globalVars);
    ap_constyp_t getApConsType(operation oper);
    void setHasChanged(string var);
    void performNECmp(string strOp1, int intOp2);
    
    void performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t abs_val);

public:
    bool operator== (const ApDomain &other) const;
    // bool operator!= (ApDomain other);
    void init(vector<string> globalVars, vector<string> functionVars);
    void copyApDomain(ApDomain copyFrom);
    // llvm::Instruction* getRelHead(string var);
    // void setRelHead(string var, llvm::Instruction *head);

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
    
    void applyInterference(string interfVar, ApDomain fromApDomain, bool isRelAcqSeq);
    void joinApDomain(ApDomain other);
    void meetApDomain(ApDomain other);
    void setVar(string strVar);
    void unsetVar(string strVar);
    bool isUnreachable();

    void addVariable(string varName);
    void printApDomain();
};

class Environment {
    REL_HEAD initRelHead(vector<string> globalVars);
    void printRelHead(REL_HEAD relHead);

    public:
    // relHead: var -> relHeadInstruction
    // environment: relHead -> ApDomain
    map <REL_HEAD, ApDomain> environment;
    
    bool operator== (const Environment &other) const;
    // map <REL_HEAD, ApDomain>::iterator begin();
    // map <REL_HEAD, ApDomain>::iterator end();

    void init(vector<string> globalVars, vector<string> functionVars);
    void copyEnvironment(Environment copyFrom);
    void addRelHead(string var, llvm::Instruction *head);
    void changeRelHeadIfNull(string var, llvm::Instruction *head);
    void changeRelHead(string var, llvm::Instruction *head);
    void changeRelHeadToNull(string var, llvm::Instruction *inst);

    // Unary Operation
    void performUnaryOp(operation oper, string strTo, string strOp);
    void performUnaryOp(operation oper, string strTo, int intOp);
    
    // Binary Operations
    void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2);
    void performBinaryOp(operation oper, string strTo, int intOp1,    string strOp2);
    void performBinaryOp(operation oper, string strTo, int intOp1,    int intOp2);
    void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2);
    
    // Other Operations
    // void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);

    // Cmp Operations
    void performCmpOp(operation oper, string strOp1, int intOp2);
    void performCmpOp(operation oper, int intOp1,    string strOp2);
    void performCmpOp(operation oper, int intOp1,    int intOp2);
    void performCmpOp(operation oper, string strOp1, string strOp2);
    
    void applyInterference(string interfVar, Environment fromEnv, bool isRelAcqSeq, llvm::Instruction *head=nullptr);
    void carryEnvironment(string interfVar, Environment fromEnv);
    void joinEnvironment(Environment other);
    void meetEnvironment(Environment other);
    void setVar(string strVar);
    void unsetVar(string strVar);
    bool isUnreachable();

    void printEnvironment();


};

#endif