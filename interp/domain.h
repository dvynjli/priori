#ifndef __DOMAIN
#define __DOMAIN

#include "common.h"

#include "ap_abstract1.h"
#include "ap_manager.h"
#include "box.h"
#include "oct.h"
#include "ap_global1.h"

#include "llvm/Support/raw_ostream.h"

#include "z3_handler.h"
#include "partial_order.h"

typedef map <string, llvm::Instruction*> REL_HEAD;
typedef map <string, PartialOrder*> POMO;

extern llvm::cl::opt<DomainTypes> AbsDomType;

class ApDomain {
    ap_manager_t *man;
    ap_environment_t *env;
    ap_abstract1_t absValue;
    // map<string, llvm::Instruction*> relHead;
    map<string, bool> hasChanged;
    
    ap_manager_t* initApManager();
    ap_environment_t* initEnvironment(vector<string> globalVars, vector<string> functionVars);
    void assignZerosToGlobals(vector<string> globalVars);
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
    
    void applyInterference(string interfVar, ApDomain fromApDomain, bool isRelAcqSync);
    void joinApDomain(ApDomain other);
    void meetApDomain(ApDomain other);
    void setVar(string strVar);
    void unsetVar(string strVar);
    bool isUnreachable();

    void addVariable(string varName);
    void printApDomain();
};




template<typename T>
class EnvironmentBase {
public:
    virtual bool operator== (const T &other) const = 0;
    // map <REL_HEAD, ApDomain>::iterator begin();
    // map <REL_HEAD, ApDomain>::iterator end();

    virtual void init(vector<string> globalVars, vector<string> functionVars) = 0;
    virtual void copyEnvironment(T copyFrom) = 0;

    // Unary Operation
    virtual void performUnaryOp(operation oper, string strTo, string strOp) = 0;
    virtual void performUnaryOp(operation oper, string strTo, int intOp) = 0;
    
    // Binary Operations
    virtual void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2) = 0;
    virtual void performBinaryOp(operation oper, string strTo, int intOp1,    string strOp2) = 0;
    virtual void performBinaryOp(operation oper, string strTo, int intOp1,    int intOp2) = 0;
    virtual void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2) = 0;
    
    // Other Operations
    // void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);

    // Cmp Operations
    virtual void performCmpOp(operation oper, string strOp1, int intOp2) = 0;
    virtual void performCmpOp(operation oper, int intOp1,    string strOp2) = 0;
    virtual void performCmpOp(operation oper, int intOp1,    int intOp2) = 0;
    virtual void performCmpOp(operation oper, string strOp1, string strOp2) = 0;
    
    virtual void applyInterference(string interfVar, T fromEnv, bool isRelAcqSync, Z3Minimal &zHelper, llvm::Instruction *interfInst=nullptr, llvm::Instruction *curInst=nullptr) = 0;
    virtual void carryEnvironment(string interfVar, T fromEnv) = 0;
    virtual void joinEnvironment(T other) = 0;
    virtual void meetEnvironment(Z3Minimal &zHelper, T other) = 0;
    virtual bool isUnreachable() = 0;

    virtual void printEnvironment() = 0;
};



class EnvironmentRelHead : public EnvironmentBase<EnvironmentRelHead> {
    REL_HEAD initRelHead(vector<string> globalVars);
    
    void printRelHead(REL_HEAD relHead);
    void addRelHead(string var, llvm::Instruction *head);
    void changeRelHead(string var, llvm::Instruction *head);

public:
    // relHead: var -> relHeadInstruction
    // environment: relHead -> ApDomain
    map <REL_HEAD, ApDomain> environment;

    void changeRelHeadToNull(string var, llvm::Instruction *inst);
    void changeRelHeadIfNull(string var, llvm::Instruction *head);
    
    virtual bool operator== (const EnvironmentRelHead &other) const;
    // map <REL_HEAD, ApDomain>::iterator begin();
    // map <REL_HEAD, ApDomain>::iterator end();

    virtual void init(vector<string> globalVars, vector<string> functionVars);
    virtual void copyEnvironment(EnvironmentRelHead copyFrom);

    // Unary Operation
    virtual void performUnaryOp(operation oper, string strTo, string strOp);
    virtual void performUnaryOp(operation oper, string strTo, int intOp);
    
    // Binary Operations
    virtual void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2);
    virtual void performBinaryOp(operation oper, string strTo, int intOp1,    string strOp2);
    virtual void performBinaryOp(operation oper, string strTo, int intOp1,    int intOp2);
    virtual void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2);
    
    // Other Operations
    // void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);

    // Cmp Operations
    virtual void performCmpOp(operation oper, string strOp1, int intOp2);
    virtual void performCmpOp(operation oper, int intOp1,    string strOp2);
    virtual void performCmpOp(operation oper, int intOp1,    int intOp2);
    virtual void performCmpOp(operation oper, string strOp1, string strOp2);
    
    virtual void applyInterference(string interfVar, EnvironmentRelHead fromEnv, bool isRelAcqSync, Z3Minimal &zHelper, llvm::Instruction *interfInst=nullptr, llvm::Instruction *curInst=nullptr);
    virtual void carryEnvironment(string interfVar, EnvironmentRelHead fromEnv);
    virtual void joinEnvironment(EnvironmentRelHead other);
    virtual void meetEnvironment(Z3Minimal &zHelper, EnvironmentRelHead other);
    void setVar(string strVar);
    void unsetVar(string strVar);
    virtual bool isUnreachable();

    virtual void printEnvironment();
};



class EnvironmentPOMO : public EnvironmentBase<EnvironmentPOMO> {
    POMO initPOMO(vector<string> globalVars);
    
    void printPOMO(POMO pomo);
    void joinPOMO (Z3Minimal &zHelper, POMO pomo1, POMO pomo2, POMO joinedPOMO);
    
    map<POMO, ApDomain>::iterator begin();
	map<POMO, ApDomain>::iterator end();

public:
    // relHead: var -> relHeadInstruction
    // environment: relHead -> ApDomain
    map <POMO, ApDomain> environment;

    // void changeRelHeadToNull(string var, llvm::Instruction *inst);
    // void changeRelHeadIfNull(string var, llvm::Instruction *head);
    
    virtual bool operator== (const EnvironmentPOMO &other) const;
    // map <REL_HEAD, ApDomain>::iterator begin();
    // map <REL_HEAD, ApDomain>::iterator end();

    virtual void init(vector<string> globalVars, vector<string> functionVars);
    virtual void copyEnvironment(EnvironmentPOMO copyFrom);

    // Unary Operation
    virtual void performUnaryOp(operation oper, string strTo, string strOp);
    virtual void performUnaryOp(operation oper, string strTo, int intOp);
    
    // Binary Operations
    virtual void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2);
    virtual void performBinaryOp(operation oper, string strTo, int intOp1,    string strOp2);
    virtual void performBinaryOp(operation oper, string strTo, int intOp1,    int intOp2);
    virtual void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2);
    
    // Other Operations
    // void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);

    // Cmp Operations
    virtual void performCmpOp(operation oper, string strOp1, int intOp2);
    virtual void performCmpOp(operation oper, int intOp1,    string strOp2);
    virtual void performCmpOp(operation oper, int intOp1,    int intOp2);
    virtual void performCmpOp(operation oper, string strOp1, string strOp2);
    
    virtual void applyInterference(string interfVar, EnvironmentPOMO fromEnv, bool isRelAcqSync, Z3Minimal &zHelper, llvm::Instruction *interfInst=nullptr, llvm::Instruction *curInst=nullptr);
    virtual void carryEnvironment(string interfVar, EnvironmentPOMO fromEnv);
    virtual void joinEnvironment(EnvironmentPOMO other);
    virtual void meetEnvironment(Z3Minimal &zHelper, EnvironmentPOMO other);
    virtual bool isUnreachable();

    virtual void printEnvironment();
};

#endif