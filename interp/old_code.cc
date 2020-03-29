// domain.h
typedef map <string, InstNum> REL_HEAD;

class EnvironmentRelHead : public EnvironmentBase<EnvironmentRelHead> {
    REL_HEAD initRelHead(vector<string> globalVars);
    
    void printRelHead(REL_HEAD relHead);
    void addRelHead(string var, InstNum &head);
    void changeRelHead(string var, InstNum &head);
    void changeRelHeadIfNull(string var, InstNum &head);

public:
    // relHead: var -> relHeadInstruction
    // environment: relHead -> ApDomain
    map <REL_HEAD, ApDomain> environment;

    void changeRelHeadToNull(string var, InstNum &inst);
    
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

    // Store Operations
    virtual void performStoreOp(InstNum &storeInst, string destVarName, Z3Minimal &zHelper);

    // Thread Join Operation
    // Perform join only for the list of variables passed in arg2
     virtual void joinOnVars(EnvironmentRelHead other, vector<string> vars, Z3Minimal &zHelper);
    // Thread Create Operation
    // Perform copy only for the list of variables passed in arg2
    virtual void copyOnVars(EnvironmentRelHead other, vector<string> vars);
    
    virtual void applyInterference(string interfVar, EnvironmentRelHead fromEnv, Z3Minimal &zHelper, 
                InstNum &curInst, InstNum &interfInst);
    virtual void carryEnvironment(string interfVar, EnvironmentRelHead fromEnv);
    virtual void joinEnvironment(EnvironmentRelHead other);
    virtual void meetEnvironment(Z3Minimal &zHelper, EnvironmentRelHead other);
    virtual void setVar(string strVar);
    virtual void unsetVar(string strVar);
    virtual bool isUnreachable();

    virtual void printEnvironment();
};


// domain.cpp

//////////////////////////////////////
//      class EnvironmentRelHead    //
//////////////////////////////////////


bool EnvironmentRelHead::operator== (const EnvironmentRelHead &other) const {
    return environment==other.environment;
}

// map <REL_HEAD, ApDomain>::iterator EnvironmentRelHead::begin() {
//     return begin();
// }

// map <REL_HEAD, ApDomain>::iterator EnvironmentRelHead::end() {
//     return end();
// }

void EnvironmentRelHead::init(vector<string> globalVars, vector<string> functionVars){
    REL_HEAD relHead = initRelHead(globalVars);
    ApDomain dom;
    dom.init(globalVars, functionVars);
    // fprintf(stderr, "dom done. assign to env\n");
    environment[relHead] = dom;
    // printEnvironment();
}

void EnvironmentRelHead::copyEnvironment(EnvironmentRelHead copyFrom){
    // environment = copyFrom.environment;
    environment.clear();
    for (auto it=copyFrom.environment.begin(); it!=copyFrom.environment.end(); ++it) {
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);
        environment[it->first]=newDomain;
    }
}

REL_HEAD EnvironmentRelHead::initRelHead(vector<string> globalVars) {
    REL_HEAD relHead;
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it) {
        relHead[(*it)] = InstNum();
    }
    return relHead;
}

// InstNum EnvironmentRelHead::getRelHead(string var) {
//     return relHead[var];
// }

void EnvironmentRelHead::addRelHead(string var, InstNum &head) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        relHead[var] = head;
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);
        auto searchRelHead = environment.find(relHead);
        if (searchRelHead != environment.end()) {
            newDomain.joinApDomain(searchRelHead->second);
        }
        environment[relHead] = newDomain;
    }
}

void EnvironmentRelHead::changeRelHeadIfNull(string var, InstNum &head) {
    map <REL_HEAD, ApDomain> newEnvironment;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        if (relHead[var] == InstNum())
            relHead[var] = head;
        newEnvironment[relHead] = it->second;
    }
    environment = newEnvironment;
}

void EnvironmentRelHead::changeRelHead(string var, InstNum &head) {
    map <REL_HEAD, ApDomain> newEnvironment;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        relHead[var] = head;
        newEnvironment[relHead] = it->second;
    }
    environment = newEnvironment;
}

void EnvironmentRelHead::changeRelHeadToNull(string var, InstNum &inst) {
    map <REL_HEAD, ApDomain> newEnvironment;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        // RelSequence terminated when a relaxed write from different thread is occured
        // relHead is changed to null only if existing relHead is from different thread
        if (relHead[var] != InstNum() && 
            getInstByInstNum(inst)->getFunction() != getInstByInstNum(relHead[var])->getFunction())
            relHead[var] = InstNum();
        newEnvironment[relHead] = it->second;
    }
    environment = newEnvironment;
}

void EnvironmentRelHead::performUnaryOp(operation oper, string strTo, string strOp) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performUnaryOp(oper, strTo, strOp);
    }
}

void EnvironmentRelHead::performUnaryOp(operation oper, string strTo, int intOp) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performUnaryOp(oper, strTo, intOp);
    }
}

void EnvironmentRelHead::performBinaryOp(operation oper, string strTo, string strOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, strOp1, intOp2);
    }
}

void EnvironmentRelHead::performBinaryOp(operation oper, string strTo, int intOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, intOp1, strOp2);
    }
}

void EnvironmentRelHead::performBinaryOp(operation oper, string strTo, int intOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, intOp1, intOp2);
    }
}

void EnvironmentRelHead::performBinaryOp(operation oper, string strTo, string strOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, strOp1, strOp2);
    }
}

// template <class OP1, class OP2>
// void EnvironmentRelHead::performCmpOp(operation oper, OP1 op1, OP2 op2) {
//     for (auto it=environment.begin(); it!=environment.end(); ++it) {
//         it->second.performCmpOp(oper, op1, op2);
//     }
// }

void EnvironmentRelHead::performCmpOp(operation oper, string strOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, strOp1, intOp2);
    }
}

void EnvironmentRelHead::performCmpOp(operation oper, int intOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, intOp1, strOp2);
    }
}

void EnvironmentRelHead::performCmpOp(operation oper, int intOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, intOp1, intOp2);
    }
}

void EnvironmentRelHead::performCmpOp(operation oper, string strOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, strOp1, strOp2);
    }
}

void EnvironmentRelHead::performStoreOp(InstNum &storeInst, string destVarName, Z3Minimal &zHelper) {
    // if (getRelHead(destVarName) == nullptr)
    //     setRelHead(destVarName, storeInst);
    changeRelHeadIfNull(destVarName, storeInst);
}

void EnvironmentRelHead::joinOnVars(EnvironmentRelHead other, vector<string> vars, Z3Minimal &zHelper) {

}

void EnvironmentRelHead::copyOnVars(EnvironmentRelHead other, vector<string> vars) {

}

void EnvironmentRelHead::applyInterference(
    string interfVar, 
    EnvironmentRelHead fromEnv, 
    Z3Minimal &zHelper, 
    InstNum &curInst,
    InstNum &interfInst
) {
    // fprintf(stderr, "Env before applying interf:\n");
    // printEnvironment();

    // if (isSyncWith) {
        carryEnvironment(interfVar, fromEnv);
    // }
    #ifdef NOTRA
    else {
        for (auto it=environment.begin(); it!=environment.end(); ++it) {
            REL_HEAD curRelHead = it->first;
            
            bool apply = true;
            for (auto relHeadIt=curRelHead.begin(); relHeadIt!=curRelHead.end(); ++relHeadIt) {
                if (relHeadIt->second != nullptr && zHelper.querySB(interfInst, relHeadIt->second)) {
                    apply = false;
                    break;
                }
            }
            if(!apply) continue;

            ApDomain curDomain, tmpDomain;
            curDomain.copyApDomain(it->second);
            for (auto interfItr=fromEnv.environment.begin(); interfItr!=fromEnv.environment.end(); ++interfItr) {
                tmpDomain.copyApDomain(it->second);
                tmpDomain.applyInterference(interfVar, interfItr->second, isSyncWith, false);
                curDomain.joinApDomain(tmpDomain);
            }
            environment[curRelHead] = curDomain;
        }
    }
    #endif
}

void EnvironmentRelHead::carryEnvironment(string interfVar, EnvironmentRelHead fromEnv) {
    map <REL_HEAD, ApDomain> newEnvironment;
        for (auto interfItr=fromEnv.environment.begin(); interfItr!=fromEnv.environment.end(); ++interfItr) {
            for (auto curItr=environment.begin(); curItr!=environment.end(); ++curItr) {
                REL_HEAD curRelHead(curItr->first);
                REL_HEAD interfRelHead(interfItr->first);
                curRelHead[interfVar] = interfRelHead[interfVar];
                ApDomain newDomain;
                newDomain.copyApDomain(curItr->second);
                newDomain.applyInterference(interfVar, interfItr->second, false);
                auto searchRelHead = environment.find(curRelHead);
                // if (searchRelHead != environment.end()) {
                //     newDomain.joinApDomain(searchRelHead->second);
                // }
                newEnvironment[curRelHead] = newDomain;
            }
        }
        environment = newEnvironment;
}

void EnvironmentRelHead::joinEnvironment(EnvironmentRelHead other) {
    for (auto it=other.environment.begin(); it!=other.environment.end(); ++it) {
        REL_HEAD relHead = it->first;
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);

        // if the relHead already exist in the current enviornment,
        // join it with the existing one
        // else add it to the current environment
        auto searchRelHead = environment.find(relHead);
        if (searchRelHead != environment.end()) {
            newDomain.joinApDomain(searchRelHead->second);
        }
        environment[relHead] = newDomain;
    }
}

void EnvironmentRelHead::meetEnvironment(Z3Minimal &zHelper, EnvironmentRelHead other) {
    for (auto it=other.environment.begin(); it!=other.environment.end(); ++it) {
        REL_HEAD relHead = it->first;
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);

        // if the relHead already exist in the current enviornment,
        // meet it with the existing one
        // else add it to the current environment
        auto searchRelHead = environment.find(relHead);
        if (searchRelHead != environment.end()) {
            newDomain.meetApDomain(searchRelHead->second);
        }
        environment[relHead] = newDomain;
    }
}

void EnvironmentRelHead::setVar(string strVar) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.setVar(strVar);
    }
}

void EnvironmentRelHead::unsetVar(string strVar) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.unsetVar(strVar);
    }
}

bool EnvironmentRelHead::isUnreachable() {
    bool isUnreach = true;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        if (!it->second.isUnreachable()) {
            isUnreach = false;
            break;
        }
    }
    return isUnreach;
}

void EnvironmentRelHead::printEnvironment() {
    fprintf(stderr, "\n--Environment--\n");
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead = it->first;
        fprintf (stderr, "RelHead:\n");
        printRelHead(relHead);
        it->second.printApDomain();
        fprintf(stderr, "\n");
    }
}

void EnvironmentRelHead::printRelHead(REL_HEAD relHead) {
    for (auto it=relHead.begin(); it!=relHead.end(); ++it) {
        fprintf(stderr, "%s: ", it->first.c_str());
        if (it->second != InstNum())
            fprintf(stderr, "%s", it->second.toString().c_str());
        else fprintf(stderr, "NULL");
        fprintf(stderr, "\n");
    }
}

