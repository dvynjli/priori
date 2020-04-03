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

// Used to represent the 
// DONOTHING: if the last write instruction of curret thread comes 
//      after last write instuction of interfering thread in POMO
// MERGE: merges both domains. if there is no ordering between last writes 
//      of current and interferring thread in POMO
// COPY: copies the domain of interfinst, if the last write of interfering 
//      thread comes after last write of current thread in POMO
enum options {UNKNOWN, DONOTHING, MERGE, COPY};

// typedef unordered_map <string, PartialOrderWrapper> POMO;

extern llvm::cl::opt<DomainTypes> AbsDomType;

class ApDomain {
    ap_manager_t *man;
    ap_environment_t *env;
    ap_abstract1_t absValue;
    // map<string, InstNum> relHead;
    map<string, bool> hasChanged;
    
    ap_manager_t* initApManager();
    ap_environment_t* initEnvironment(vector<string> &globalVars, vector<string> &functionVars);
    void assignZerosToGlobals(vector<string> &globalVars);
    void initHasChanged(vector<string> &globalVars);
    ap_constyp_t getApConsType(operation oper);
    void setHasChanged(string var);
    void performNECmp(string strOp1, int intOp2);
    void performNECmp(string strOp1, string strOp2);
    
    void performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t &abs_val);
    // InstNum getRelHead(string var);
    // void setRelHead(string var, InstNum head);
    void copyVar(ApDomain &fromApDomain, ap_var_t apVar);
    void joinVar(ApDomain &fromApDomain, ap_var_t apVar);

public:
    bool operator== (const ApDomain &other) const;
    // bool operator!= (ApDomain other);
    void init(vector<string> &globalVars, vector<string> &functionVars);
    void copyApDomain(ApDomain &copyFrom);

    // Unary Operations
    void performUnaryOp(operation oper, string strTo, string strOp);
    void performUnaryOp(operation oper, string strTo, int intOp);
    // Binary Operations and RMW Operations
    void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2);
    void performBinaryOp(operation oper, string strTo, int intOp1,    string strOp2);
    void performBinaryOp(operation oper, string strTo, int intOp1,    int intOp2);
    void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2);
    // Cmp Operations
    void performCmpOp(operation oper, string strOp1, int intOp2);
    void performCmpOp(operation oper, int intOp1,    string strOp2);
    void performCmpOp(operation oper, int intOp1,    int intOp2);
    void performCmpOp(operation oper, string strOp1, string strOp2);
    // Other Operations
    void performCmpXchgOp(string strTo, string strCmpVal, string strNewVal);
    
    // Perform join only for the list of variables passed in arg2
    void joinOnVars(ApDomain &other, vector<string> &vars);
    // Perform join only for the list of variables passed in arg2
    void copyOnVars(ApDomain &other, vector<string> &vars);


    void applyInterference(string interfVar, ApDomain &fromApDomain, bool isPOMO, map<string, options> *varoptions=nullptr);
    void joinApDomain(ApDomain &other);
    void meetApDomain(ApDomain &other);
    void setVar(string strVar);
    void unsetVar(string strVar);
    bool isUnreachable();

    void addVariable(string varName);
    void printApDomain();
};




template<typename T>
class EnvironmentBase {
    bool modified = true;
public:
    virtual bool operator== (const T &other) const = 0;

    void setModified() {
        modified = true;
    }

    void setNotModified() {
        modified = false;
    }

    bool isModified() {
        return modified;
    }

    // map <REL_HEAD, ApDomain>::iterator begin();
    // map <REL_HEAD, ApDomain>::iterator end();

    virtual void init(vector<string> &globalVars, vector<string> &functionVars) = 0;
    virtual void copyEnvironment(T &copyFrom) = 0;

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

    // Store Operation
    virtual void performStoreOp(InstNum &storeInst, string destVarName, Z3Minimal &zHelper)=0;

    // Thread Join Operation
    // Perform join only for the list of variables passed in arg2
     virtual void joinOnVars(T &other, vector<string> &vars, Z3Minimal &zHelper)=0;
    // Thread Create Operation
    // Perform copy only for the list of variables passed in arg2
     virtual void copyOnVars(T &other, vector<string> &vars)=0;
    
    /** Updates the abstract domain of current instruction as per the interferring domain. Argurments are
        * interfVar: Variable on which interference is happening
        * interfEnv: Environment of interfering instruction
        * isSyncWith: True for release-acquire synchronization, false otherwise
        * zHelper: Instance of z3 to be used to check sequences-before
        * interfInst: Interferring instruction
        * curInst: Current Instruction
        */
    virtual void applyInterference(string interfVar, T &interfEnv, Z3Minimal &zHelper, 
                InstNum &curInst, InstNum &interfInst) = 0;
    virtual void carryEnvironment(string interfVar, T &fromEnv) = 0;
    virtual void joinEnvironment(T &other) = 0;
    virtual void meetEnvironment(Z3Minimal &zHelper, T &other) = 0;
    virtual void setVar(string strVar) = 0;
    virtual void unsetVar(string strVar) = 0;
    virtual bool isUnreachable() = 0;

    virtual void printEnvironment() = 0;
};

class POMO {
public:
    unordered_map <string, PartialOrderWrapper> pomo;
    unordered_map <string, PartialOrderWrapper>::const_iterator begin() const {
        return pomo.begin();
    }
	unordered_map <string, PartialOrderWrapper>::const_iterator end() const {
        return pomo.end();
    }
    unordered_map <string, PartialOrderWrapper>::iterator find(string var) {
        return pomo.find(var);
    }
    bool empty() {
        return pomo.empty();
    }


    void emplace(string var, PartialOrderWrapper po) {
        pomo.emplace(var, po);
    }

    bool operator== (const POMO &other) const {
        for (auto it: pomo) {
            auto searchOther = other.find(it.first);
            assert(searchOther != other.end()
                    && "other pomo does not contain variable");
            if (!(it.second == searchOther->second)) {
                return false;
            }
        }
        return true;
    }
    void operator= (const POMO &other) {
        pomo = other.pomo;
    }

    void printPOMO() {
    // fprintf(stderr, "Printing POMO\n");
    for (auto it=pomo.begin(); it!=pomo.end(); ++it) {
        fprintf(stderr, "%s, %p: ", it->first.c_str(), &(it->second));
        // if (it->second)
            fprintf(stderr, "%s\n", it->second.toString().c_str());
        // else fprintf(stderr, "NULL");
        // fprintf(stderr, "\n");
    }
    // fprintf(stderr, "printing done\n");
}
};

namespace std{
template<>
struct hash<POMO> {
	size_t operator() (const POMO &pomo) const {
		// return (hash<unsigned short>()(in.getTid()) ^ 
		auto it = pomo.begin();
		if (it == pomo.end()); {
            PartialOrder po = PartialOrder();
			return hash<PartialOrderWrapper>()(PartialOrderWrapper(&po));
        }
		size_t curhash = hash<PartialOrderWrapper>()(it->second);
		it++;
		for (; it!=pomo.end(); it++) {
			curhash = curhash ^ hash<PartialOrderWrapper>()(it->second);
		}
		// fprintf(stderr, "returning hash\n");
		return curhash;
	}
};
}

class EnvironmentPOMO : public EnvironmentBase<EnvironmentPOMO> {

    // relHead: var -> relHeadInstruction
    // environment: relHead -> ApDomain
    unordered_map <POMO, ApDomain> environment;
    
    void initPOMO(vector<string> &globalVars, POMO *pomo);
    
    void printPOMO(const POMO &pomo);
    void joinPOMO (Z3Minimal &zHelper, const POMO &pomo1, const POMO &pomo2, POMO &joinedPOMO);
    
    unordered_map<POMO, ApDomain>::iterator begin();
	unordered_map<POMO, ApDomain>::iterator end();

    // void getVarOption (map<string, options> *varoptions, string varName,PartialOrderWrapper curPartialOrder,
    //             map<InstNum, map<string, InstNum>> *lastWrites, 
    //             InstNum interfInst, InstNum curInst, Z3Minimal &zHelper);
    void getVarOption (map<string, options> *varoptions, 
                string varName,
                PartialOrderWrapper &curPartialOrder,
                PartialOrderWrapper &interfPartialOrder, 
                Z3Minimal &zHelper);

public:

    // void changeRelHeadToNull(string var, InstNum inst);
    // void changeRelHeadIfNull(string var, InstNum head);
    
    virtual bool operator== (const EnvironmentPOMO &other) const;
    // void operator= (const EnvironmentPOMO &other) {
    //     environment = other.environment;
    // }
    // map <REL_HEAD, ApDomain>::iterator begin();
    // map <REL_HEAD, ApDomain>::iterator end();

    virtual void init(vector<string> &globalVars, vector<string> &functionVars);
    virtual void copyEnvironment(EnvironmentPOMO &copyFrom);

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

    // Store Operations
    virtual void performStoreOp(InstNum &storeInst, string destVarName, Z3Minimal &zHelper);

    // Thread Join Operation
    // Perform join only for the list of variables passed in arg2
    virtual void joinOnVars(EnvironmentPOMO &other, vector<string> &vars, Z3Minimal &zHelper);
    // Thread Create Operation
    // Perform copy only for the list of variables passed in arg2
    virtual void copyOnVars(EnvironmentPOMO &other, vector<string> &vars);
    
    virtual void applyInterference(string interfVar, EnvironmentPOMO &fromEnv, Z3Minimal &zHelper, 
                InstNum &curInst, InstNum &interfInst);
    virtual void joinEnvironment(EnvironmentPOMO &other);
    virtual void meetEnvironment(Z3Minimal &zHelper, EnvironmentPOMO &other);
    // TODO: this function is not required for POMO. change the structure to use append instead of this
    virtual void carryEnvironment(string interfVar, EnvironmentPOMO &fromEnv);
    // virtual void appendInst(Z3Minimal &zHelper, llvm::StoreInst *storeInst, string var);
    virtual void setVar(string strVar);
    virtual void unsetVar(string strVar);
    virtual bool isUnreachable();

    virtual void printEnvironment();
};

#endif