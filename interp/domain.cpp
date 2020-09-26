#include "domain.h"


bool ApDomain::operator== (const ApDomain &other) const {
    ap_abstract1_t tempAbsVal = other.absValue;
    return ap_environment_is_eq(env, other.env) && 
        ap_abstract1_is_eq(man, (ap_abstract1_t *) &absValue, &tempAbsVal);
}

// bool ApDomain::operator!= (ApDomain other) {
//     return !(operator==(other));
// }

void ApDomain::init(vector<string> &globalVars, vector<string> &functionVars){
    // fprintf(stderr, "initializing ap_man\n");
    man = initApManager();
    // DEBUG && fprintf(stderr, "Init Env\n");
    env = initEnvironment(globalVars, functionVars);
    // ap_environment_fdump(stderr, env);
    // ap_var_t var = ap_environment_var_of_dim(env, 0);
    // fprintf(stderr, "var: %s\n", var);
    // DEBUG && fprintf(stderr, "creating top\n");
    absValue = ap_abstract1_top(man, env);
    assignZerosToGlobals(globalVars);
    //initRelHead(globalVars);
    initHasChanged(globalVars);
    // printApDomain();

    // DEBUG && fprintf(stderr, "performing transforms\n");
    // performTrasfer(man, env, absValue);
}

void ApDomain::initHasChanged(vector<string> &globalVars) {
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it) {
        hasChanged[(*it)] = false;
    }
}

void ApDomain::setHasChanged(const string &var) {
    auto searchHasChanged = hasChanged.find(var);
    if (searchHasChanged != hasChanged.end() && !searchHasChanged->second) {
        hasChanged[var] = true;
    }
}

void ApDomain::assignZerosToGlobals(vector<string> &globalVars) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_S_INT, 0, AP_END);
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it) {
        ap_var_t var = (ap_var_t) (it->c_str());
        absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    }
}

ap_manager_t* ApDomain::initApManager() {
    //TODO: parameterize by command line arg
    if (AbsDomType == interval)
        return box_manager_alloc();
    else if(AbsDomType == octagon)
        return oct_manager_alloc();
    else {
        fprintf(stderr, "ERROR: unkown domain\n");
        exit(0);
    }
}

ap_environment_t* ApDomain::initEnvironment(vector<string> &globalVars, vector<string> &functionVars){
    ap_var_t intAp[globalVars.size() + functionVars.size()];
    int i = 0;
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it, ++i) {
        intAp[i] = (ap_var_t)(it->c_str());
    }
    for (auto it=functionVars.begin(); it!=functionVars.end(); ++it, ++i){
        intAp[i] = (ap_var_t)(it->c_str());
    }
    // TODO: use if providing support for floats
    ap_var_t floatAp[0];
    
    //return nullptr;
    return ap_environment_alloc(intAp, globalVars.size()+functionVars.size(), floatAp, 0);
}

void ApDomain::copyApDomain(ApDomain &copyFrom) {
    man = copyFrom.man;
    env = ap_environment_copy(copyFrom.env);
    absValue = ap_abstract1_copy(man, &copyFrom.absValue);
    // relHead = copyFrom.relHead;
    hasChanged = copyFrom.hasChanged;
}

void ApDomain::joinApDomain(ApDomain &other) {
    ap_abstract1_join(man, true, &absValue, &other.absValue);
}

void ApDomain::meetApDomain(ApDomain &other){
    ap_abstract1_meet(man, true, &absValue, &other.absValue);
}

void ApDomain::addVariable(string &varName) {
    int newSize = (env->intdim) + 1;
    ap_var_t intAp[newSize];
    for (int i = 0; i < newSize-1; i++){
        intAp[i] = ap_environment_var_of_dim(env, i);
    }
    intAp[newSize-1] = (ap_var_t)(varName.c_str());
    ap_var_t floatAp[0];
    ap_environment_t *env1 = ap_environment_alloc(intAp, newSize, floatAp, 0);
    ap_abstract1_t absVal1 = ap_abstract1_change_environment(man, true, &absValue, env1, false);
    // ap_abstract1_fdump(stderr, man,  &absVal1);
    env = env1;
    absValue = absVal1;
    // env1 = ap_environment_add(env, intAp, newSize, floatAp, 0);
    // fprintf(stderr, "new env:\n");
    // ap_environment_fdump(stderr, env);
    // for (int i = 0; i < newSize-1; i++){
    //     intAp[i] = ap_environment_var_of_dim(env1, i);
    // }
    // ap_environment_fdump(stderr, env1);
}

void ApDomain::performUnaryOp(operation &oper, string &strTo, int &intOp) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    switch(oper) {
        // case LOAD is not possible
        case STORE:
            ap_linexpr1_set_list(&expr, AP_CST_S_INT, intOp, AP_END);
            break;
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
}

void ApDomain::performUnaryOp(operation &oper, string &strTo, string &strOp) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    switch(oper) {
        case LOAD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp.c_str(), AP_END);
            break;
        case STORE:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp.c_str(), AP_END);
            break;
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
}

void ApDomain::performBinaryOp(operation &oper, string &strTo, string &strOp1, string &strOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    ap_interval_t *fromInterval=nullptr; 
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, 1, strOp2.c_str(), AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, -1, strOp2.c_str(), AP_END);
            break;
        case MUL:
            // for multiplication of two variables, need to intervalize one
			fromInterval = ap_abstract1_bound_variable(man, &absValue, (ap_var_t)strOp2.c_str());
            ap_linexpr1_set_list(&expr, AP_COEFF_I, fromInterval, strOp1.c_str(), AP_END);
            break;
        default:
            fprintf(stderr, "WARNING: unsupported binary operation\n");
            break;
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
    // if (oper == ADD) {
    //     fprintf(stderr, "Var assigned: %s\n", var);
    //     fprintf(stderr, "After add on to: %s, op1: %s, op2: %s. ApDom:\n", strTo.c_str(), strOp1.c_str(), strOp2.c_str());
    //     printApDomain();
    // }
}

void ApDomain::performBinaryOp(operation &oper, string &strTo, string &strOp1, int &intOp2) {
    // fprintf(stderr, "performBinOp called str int\n");
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_CST_S_INT, intOp2, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_CST_S_INT, (-1)*intOp2, AP_END);
            break;
        case MUL:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, intOp2, strOp1.c_str(), AP_END);
            break;
        default:
            fprintf(stderr, "WARNING: unsupported binary operation\n");
            break;
    }
    // ap_linexpr1_fprint(stderr, &expr);
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
    // if (oper == ADD) {
    //     fprintf(stderr, "After add ApDom:\n");
    //     printApDomain();
    // }
}

void ApDomain::performBinaryOp(operation &oper, string &strTo, int &intOp1, string &strOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp2.c_str(), AP_CST_S_INT, intOp1, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, -1, strOp2.c_str(), AP_CST_S_INT, intOp1, AP_END);
            break;
        case MUL:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, intOp1, strOp2.c_str(), AP_END);
            break;
        default:
            fprintf(stderr, "WARNING: unsupported binary operation\n");
            break;
    }
    // ap_linexpr1_fprint(stderr, &expr);
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
    // if (oper == ADD) {
    //     fprintf(stderr, "After add ApDom:\n");
    //     printApDomain();
    // }
}

void ApDomain::performBinaryOp(operation &oper, string &strTo, int &intOp1, int &intOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_CST_S_INT, intOp1+intOp2, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_CST_S_INT, intOp1-intOp2, AP_END);
            break;
        case MUL:
            ap_linexpr1_set_list(&expr, AP_CST_S_INT, intOp1*intOp2, AP_END);
            break;
        case DIV:
            ap_linexpr1_set_list(&expr, AP_CST_S_INT, intOp1/intOp2, AP_END);
            break;            
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
    // if (oper == ADD) {
    //     fprintf(stderr, "After add ApDom:\n");
    //     printApDomain();
    // }
}

void ApDomain::performCmpOp(operation oper, string &strOp1, int &intOp2) {
    if (oper==LT) {
        // apron doesn't have LT cons operator. Need to change it to GT by swapping the operands.
        performCmpOp(GT, intOp2, strOp1);
        return;
    }
    else if (oper == LE) {
        // apron doesn't have LE cons operator. Need to change it to GE by swapping the operands.
        performCmpOp(GE, intOp2, strOp1);
        return;
    }
    else if (oper == NE) {
        performNECmp(strOp1, intOp2);
        return;
    }
    ap_constyp_t op = getApConsType(oper);

    // fprintf(stderr, "%d %s %d\n", oper, strOp1.c_str(), intOp2);
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    ap_lincons1_t consExpr = ap_lincons1_make(op, &expr, NULL);
    ap_lincons1_set_list(&consExpr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_CST_S_INT, (-1)*intOp2, AP_END);
    // fprintf(stderr, "ConsExpr: ");
    // ap_lincons1_fprint(stderr, &consExpr);
    ap_lincons1_array_t consArray = ap_lincons1_array_make(env, 1);
    // fprintf(stderr, "\nconsArray: ");
    ap_lincons1_array_set(&consArray, 0, &consExpr);
    // ap_lincons1_array_fprint(stderr, &consArray);
    // printApDomain();
    // fprintf(stderr, "\nmeet:\n");
    absValue = ap_abstract1_meet_lincons_array(man, true, &absValue, &consArray);
    // printApDomain();
}

void ApDomain::performCmpOp(operation oper, int &intOp1, string &strOp2) {
    if (oper==LT) {
        // apron doesn't have LT cons operator. Need to change it to GT by swapping the operands.
        performCmpOp(GT, strOp2, intOp1);
        return;
    }
    else if (oper == LE) {
        // apron doesn't have LE cons operator. Need to change it to GE by swapping the operands.
        performCmpOp(GE, strOp2, intOp1);
        return;
    }
    else if (oper == NE) {
        // NE is commutative
        performNECmp(strOp2, intOp1);
        return;
    }
    ap_constyp_t op = getApConsType(oper);
    // fprintf(stderr, "%d %d %s\n", oper, intOp1, strOp2.c_str());
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    ap_lincons1_t consExpr = ap_lincons1_make(op, &expr, NULL);
    ap_lincons1_set_list(&consExpr, AP_COEFF_S_INT, -1, strOp2.c_str(), AP_CST_S_INT, intOp1, AP_END);
    // fprintf(stderr, "ConsExpr: ");
    // ap_lincons1_fprint(stderr, &consExpr);
    ap_lincons1_array_t consArray = ap_lincons1_array_make(env, 1);
    // fprintf(stderr, "\nconsArray: ");
    ap_lincons1_array_set(&consArray, 0, &consExpr);
    // ap_lincons1_array_fprint(stderr, &consArray);
    // printApDomain();
    // fprintf(stderr, "\nmeet:\n");
    absValue = ap_abstract1_meet_lincons_array(man, true, &absValue, &consArray);
    // printApDomain();
}

void ApDomain::performCmpOp(operation oper, int &intOp1, int &intOp2) {
    // never occurs
    fprintf(stderr, "ERROR: performCmpOp() with both operand of condition as constant. This function should never called!!");
    exit(0);
}

void ApDomain::performCmpOp(operation oper, string &strOp1, string &strOp2) {
    if (oper==LT) {
        // apron doesn't have LT cons operator. Need to change it to GT by swapping the operands.
        performCmpOp(GT, strOp2, strOp1);
        return;
    }
    else if (oper == LE) {
        // apron doesn't have LE cons operator. Need to change it to GE by swapping the operands.
        performCmpOp(GE, strOp2, strOp1);
        return;
    }
    else if (oper == NE) {
        // NE is commutative
        performNECmp(strOp2, strOp1);
        return;
    }
    ap_constyp_t op = getApConsType(oper);
    // fprintf(stderr, "%d %d %s\n", oper, intOp1, strOp2.c_str());
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    ap_lincons1_t consExpr = ap_lincons1_make(op, &expr, NULL);
    ap_lincons1_set_list(&consExpr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, -1, strOp2.c_str(), AP_END);
    // fprintf(stderr, "ConsExpr: ");
    // ap_lincons1_fprint(stderr, &consExpr);
    ap_lincons1_array_t consArray = ap_lincons1_array_make(env, 2);
    // fprintf(stderr, "\nconsArray: ");
    ap_lincons1_array_set(&consArray, 0, &consExpr);
    // ap_lincons1_array_fprint(stderr, &consArray);
    // printApDomain();
    // fprintf(stderr, "\nmeet:\n");
    absValue = ap_abstract1_meet_lincons_array(man, true, &absValue, &consArray);
    // printApDomain();
}

// Perform join only for the list of variables passed in arg2
void ApDomain::joinOnVars(ApDomain &other, vector<string> &vars) {
    for (auto var: vars) {
        ap_var_t apVar = (ap_var_t) var.c_str();
        joinVar(other, apVar);
    }
}

void ApDomain::copyOnVars(ApDomain &other, vector<string> &vars) {
    for (auto var: vars) {
        ap_var_t apVar = (ap_var_t) var.c_str();
        copyVar(other, apVar);
    }
}

void ApDomain::printApDomain() {
    ap_abstract1_fprint(stderr, man,  &absValue);
}

void ApDomain::applyInterference(string interfVar, ApDomain &fromApDomain, bool isPOMO, 
            map<string, options> *varoptions
) {
    ap_var_t apInterVar;

    if(isPOMO) {
        // use varoptions to determine what to do for each variable
        for (auto it:(*varoptions)) {
            ap_var_t apVar = (ap_var_t) it.first.c_str();
            if (it.second == COPY || it.first == interfVar) {
                // copy the variable from the fromApDomain
                copyVar(fromApDomain, apVar);
            }
            else if (it.second == MERGE) {
                // merge the domain for this variable
                joinVar(fromApDomain, apVar);
            }
            // else DONOTHING 
        }
    }

    // If this is a part of release-acquire sequence, copy the values of all the global vars,
    // else only the variable for which interference is 
    else {
    // if (isSyncWith) { // since memory model is RA, it is not required
        // fprintf(stderr, "applyinterf in ApDom for var %s. Dom before apply:\n", interfVar.c_str());
        // printApDomain();

        for (auto it=hasChanged.begin(); it!=hasChanged.end(); ++it) {
            apInterVar = (ap_var_t) it->first.c_str();
            if (ap_environment_dim_of_var(env, apInterVar) == AP_DIM_MAX) {
                fprintf(stderr, "ERROR: Interfering variable not in domain. Something went wrong.\n");
                exit(0);
            }
            if (it->second && it->first!=interfVar) {
                // there has been some event that has initialized the variable. 
                // Need to join the environment of this variable
                joinVar(fromApDomain, apInterVar);
                // ap_abstract1_t tmpValue = ap_abstract1_copy(man, &absValue);
                
                // // initialize it with the value of variable to be joined
                // ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apInterVar);
                // ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
                // ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
                // tmpValue = ap_abstract1_assign_linexpr(man, true, &tmpValue, apInterVar, &expr, NULL);
                
                // // join the two abstract values
                // absValue =  ap_abstract1_join(man, true, &tmpValue, &absValue);
            }
            else {
                // the variable is unintialized. Copy from the fromApDomain
                copyVar(fromApDomain, apInterVar);
                // ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apInterVar);
                // ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
                // ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
                // absValue = ap_abstract1_assign_linexpr(man, true, &absValue, apInterVar, &expr, NULL);
                setHasChanged(it->first);
            }
        }
        // fprintf(stderr, "dom after apply:\n");
        // printApDomain();
    }

    #ifdef NOTRA
    // since memory model is RA, it is not required
    else {
        apInterVar = (ap_var_t) interfVar.c_str();
        if (ap_environment_dim_of_var(env, apInterVar) == AP_DIM_MAX) {
            fprintf(stderr, "ERROR: Interfering variable not in domain. Something went wrong.\n");
            exit(0);
        }
        // ap_lincons1_array_t arr = ap_abstract1_to_lincons_array(fromApDomain.man, &fromApDomain.absValue);
        // fprintf(stderr, "array: ");
        // ap_lincons1_array_fprint(stderr, &arr);
        // absValue = ap_abstract1_of_lincons_array(man, env, &arr);

        // fprintf(stderr, "Appling insterf from domain: \n");
        // fromApDomain.printApDomain();
        // fprintf(stderr, "To domain: \n");
        // printApDomain();
        // fprintf(stderr, "On var %s\n", interfVar.c_str());
        copyVar(fromApDomain, apInterVar);
        // ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apInterVar);
        // // ap_interval_fprint(stderr, fromInterval);
        // ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
        // ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
        // absValue = ap_abstract1_assign_linexpr(man, true, &absValue, apInterVar, &expr, NULL);
        setHasChanged(interfVar);
    }
    #endif

    // fprintf(stderr, "ApDomain after interf:\n");
    // printApDomain();
}

void ApDomain::setVar(string &strVar) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_S_INT, 1, AP_END);
    ap_var_t var = (ap_var_t) strVar.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void ApDomain::unsetVar(string &strVar) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_S_INT, 0, AP_END);
    ap_var_t var = (ap_var_t) strVar.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

bool ApDomain::isUnreachable() {
    return ap_abstract1_is_bottom(man, &absValue);
}

ap_constyp_t ApDomain::getApConsType(operation oper) {
    switch (oper) {
        case EQ:
            return AP_CONS_EQ;
        case NE:
            return AP_CONS_DISEQ;
        case GT:
            return AP_CONS_SUP;
        case GE:
            return AP_CONS_SUPEQ;
		default:
			fprintf(stderr, "Not valid operation\n");
			exit(0);
    }
}

void ApDomain::performNECmp(string &strOp1, int &intOp2) {
    // ltDomain = strOp1 < intOp2
    ApDomain ltDomain;
    ltDomain.copyApDomain(*this);
    ltDomain.performCmpOp(LT, strOp1, intOp2);
    // this = strOp1 > intOp1
    performCmpOp(GT, strOp1, intOp2);
    // this = ltDomain join this
    joinApDomain(ltDomain);
}

void ApDomain::performNECmp(string &strOp1, string &strOp2) {
    // ltDomain = strOp1 < intOp2
    ApDomain ltDomain;
    ltDomain.copyApDomain(*this);
    ltDomain.performCmpOp(LT, strOp1, strOp2);
    // this = strOp1 > intOp1
    performCmpOp(GT, strOp1, strOp2);
    // this = ltDomain join this
    joinApDomain(ltDomain);
}

void ApDomain::performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t &absValue) {
    /* assign x = 1 */
    // fprintf(stderr, "Assigning x = 1\n");
    // ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    // ap_linexpr1_set_list(&expr, AP_CST_S_INT, 1, AP_END);
    // ap_linexpr1_fprint(stderr, &expr);
    // ap_var_t var = ap_environment_var_of_dim(env, 0);
    // absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    // fprintf(stderr, "assigned linexpr to x\n");
    // ap_abstract1_fprint(stderr, man, &absValue);
    // // ap_linexpr1_clear(&expr);

    // /* assign y = x + 10 */
    // fprintf(stderr, "Assigning y = x + 10\n");
    // ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, "x", AP_CST_S_INT, 10, AP_END);
    // ap_linexpr1_fprint(stderr, &expr);
    // var = ap_environment_var_of_dim(env, 1);
    // absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    // fprintf(stderr, "assigned linexpr to x\n");
    // ap_abstract1_fprint(stderr, man, &absValue);
    // // ap_linexpr1_clear(&expr);

    // /* assign x = x + y */
    // fprintf(stderr, "Assigning x = x + y\n");
    // ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, "x", AP_COEFF_S_INT, 1, "y", AP_CST_S_INT, 0, AP_END);
    // ap_linexpr1_fprint(stderr, &expr);
    // var = ap_environment_var_of_dim(env, 0);
    // absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    // fprintf(stderr, "assigned linexpr to x\n");
    // ap_abstract1_fprint(stderr, man, &absValue);
    
    // ap_linexpr1_clear(&expr);
}

// copy the variable from the fromApDomain
void ApDomain::copyVar(ApDomain &fromApDomain, ap_var_t &apVar) {
    ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apVar);
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, apVar, &expr, NULL);
}

void ApDomain::joinVar(ApDomain &fromApDomain, ap_var_t &apVar) {
    ap_abstract1_t tmpValue = ap_abstract1_copy(man, &absValue);
                
    // initialize tmp domain with the value of variable to be joined
    ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apVar);
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
    tmpValue = ap_abstract1_assign_linexpr(man, true, &tmpValue, apVar, &expr, NULL);
    
    // join the two abstract values
    absValue =  ap_abstract1_join(man, true, &tmpValue, &absValue);
}




//////////////////////////////////////
//      class EnvironmentPOMO       //
//////////////////////////////////////
bool EnvironmentPOMO::operator== (const EnvironmentPOMO &other) const {
    return environment==other.environment;
}


void EnvironmentPOMO::init(vector<string> &globalVars, 
    vector<string> &functionVars, 
    vector<string> &locks)
{
    POMO pomo;
    initPOMO(globalVars, locks, pomo);
    // fprintf(stderr, "pomo initialized\n");
    // pomo.printPOMO();
    ApDomain dom;
    dom.init(globalVars, functionVars);
    // fprintf(stderr, "dom done. assign to env\n");
    environment[pomo] = dom;
    // printEnvironment();
}

void EnvironmentPOMO::initPOMO(vector<string> &globalVars, vector<string> &locks, POMO &pomo){
    // fprintf(stderr, "decl pomo\n");
    // POMO pomo;
    PartialOrder po = PartialOrderWrapper::getEmptyPartialOrder(true);
    // fprintf(stderr, "assigning empty to pomo\n");
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it) {
        pomo.emplace((*it), po);
    }
    PartialOrder polocks = PartialOrderWrapper::getEmptyPartialOrder(false);
    for (auto it=locks.begin(); it!=locks.end(); ++it) {
        pomo.emplace((*it), polocks);
    }
    // delete po;
    // pomo.printPOMO();
    // PartialOrderWrapper::printAllPO();
    // fprintf(stderr, "returning pomo\n");
    // return pomo;
}

void EnvironmentPOMO::copyEnvironment(EnvironmentPOMO &copyFrom){
    // environment = copyFrom.environment;
    environment.clear();
    for (auto it=copyFrom.environment.begin(); it!=copyFrom.environment.end(); ++it) {
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);
        environment[it->first]=newDomain;
    }
    if (copyFrom.isModified()) setModified();
    else setNotModified();
    // fprintf(stderr, "copied from:\n");
    // copyFrom.printEnvironment();
    // fprintf(stderr, "after copying:\n");
    // printEnvironment();
}

void EnvironmentPOMO::performUnaryOp(operation oper, string strTo, string strOp) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performUnaryOp(oper, strTo, strOp);
    }
}

void EnvironmentPOMO::performUnaryOp(operation oper, string strTo, int intOp) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performUnaryOp(oper, strTo, intOp);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string &strTo, string &strOp1, int &intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, strOp1, intOp2);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string &strTo, int &intOp1, string &strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, intOp1, strOp2);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string &strTo, int &intOp1, int &intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, intOp1, intOp2);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string &strTo, string &strOp1, string &strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, strOp1, strOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, string &strOp1, int &intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, strOp1, intOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, int &intOp1, string &strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, intOp1, strOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, int &intOp1, int &intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, intOp1, intOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, string &strOp1, string &strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, strOp1, strOp2);
    }
}

void EnvironmentPOMO::performReleaseLock(string &lockVar, InstNum unlockInst) {
    unordered_map <POMO, ApDomain> newEnv;
    for (auto it=environment.begin(); it!=environment.end(); it++) {
        POMO tmpPomo=it->first;
        auto searchLockVar = tmpPomo.find(lockVar);
        assert(searchLockVar != tmpPomo.end() && "LockVar not found in POMO");

        unordered_set<InstNum> lastInPO;
        searchLockVar->second.getLasts(lastInPO);
        // last in PO for lockVar should be a lock instruction
        assert(lastInPO.size()==1 && "Last of PO for lockVar has to be a single inst");

        PartialOrder tmpPO = PartialOrderWrapper::append(searchLockVar->second, unlockInst);
        tmpPomo.emplace(lockVar, tmpPO);
        newEnv[tmpPomo] = it->second;
    }
    environment = newEnv;
}

void EnvironmentPOMO::performAcquireLock(string &lockVar, InstNum lockInst) {
    unordered_map <POMO, ApDomain> newEnv;
    for (auto it=environment.begin(); it!=environment.end(); it++) {
        POMO tmpPomo=it->first;
        // fprintf(stderr, "tmpPomo:\n");
        // tmpPomo.printPOMO();
        // fprintf(stderr, "lockVar: %s\n", lockVar.c_str());
        auto searchLockVar = tmpPomo.find(lockVar);
        assert(searchLockVar != tmpPomo.end() && "LockVar not found in POMO");

        // fprintf(stderr, "calling toString\n");
        // fprintf(stderr, "PO before append: %s\n", searchLockVar->second.toString().c_str());
        PartialOrder tmpPO = PartialOrderWrapper::append(searchLockVar->second, lockInst);
        // fprintf(stderr, "PO after append: %s\n", tmpPO.toString().c_str());

        tmpPomo.emplace(lockVar, tmpPO);
        newEnv[tmpPomo] = it->second;
    }
    environment = newEnv;
}

void EnvironmentPOMO::performStoreOp(InstNum &storeInst, string &destVarName) {
    unordered_map <POMO, ApDomain> newEnv;
    for (auto it: environment) {
        POMO tmpPomo=it.first;
        // for (auto varIt: it.first) {
        //     tmpPomo[varIt.first] = varIt.second;
        //     if (varIt.first == destVarName) {
        //         // fprintf(stderr, "appending from POMO\n");
        //         tmpPomo[varIt.first].append(zHelper, storeInst);
        //     }
        // }
        auto searchDestVar = tmpPomo.find(destVarName);
        assert(searchDestVar != tmpPomo.end() && "variable does not exists in envrionment POMO");
        PartialOrder tmpPO = PartialOrderWrapper::append(searchDestVar->second, storeInst);
        tmpPomo.emplace(searchDestVar->first, tmpPO);
        newEnv[tmpPomo] = it.second;
        // fprintf(stderr, "aftre emplace call in performStoreOp:\n");
        // tmpPomo.printPOMO();
    }
    environment = newEnv;
}

void EnvironmentPOMO::joinOnVars(EnvironmentPOMO &other, vector<string> &vars) {
    unordered_map <POMO, ApDomain> newenvironment;
    // fprintf(stderr,"For thread join. Other:\n");
    // other.printEnvironment();
    // fprintf(stderr, "Current:\n");
    // printEnvironment();
    if (other.isUnreachable()) {
        environment.clear();
        // fprintf(stderr, "other is empty. returning\n");
        return;
    }
    
    for (auto curItr: environment) {
        // fprintf(stderr, "in curItr loop\n");
        POMO curPomo = curItr.first;
        // POMO newPomo=curPomo;
        // ApDomain tmpDomain;
        // tmpDomain.copyApDomain(curItr.second);
                
        for (auto otherItr: other) {
            POMO otherPomo = otherItr.first;
            ApDomain tmpDomain;
            tmpDomain.copyApDomain(curItr.second);
            POMO newPomo = curPomo;

            bool apply = true;
            for (auto varItr: curPomo) {
                auto searchVarOtherItr = otherPomo.find(varItr.first);
                if (searchVarOtherItr == otherPomo.end()) {
                    fprintf(stderr, "ERROR: Variable %s mismatch in POMOs in joinOnVars\n", varItr.first.c_str());
                    fprintf(stderr, "Was joining with:\n");
                    otherPomo.printPOMO();
                    fprintf(stderr, "other.first:\n");
                    otherItr.first.printPOMO();
                    exit(0);
                }
                if (!varItr.second.isConsistent(searchVarOtherItr->second)) {
                    apply = false;
                    // fprintf(stderr, "Inconsistent POMOs on var %s\n", varItr.first.c_str());
                    // curPomo.printPOMO();otherPomo.printPOMO();
                    break;
                }
                if (!varItr.second.isConsistentRMW(searchVarOtherItr->second)) {
                    apply=false;
                    break;
                }
            }

            if (apply) {
                map<string, options> varoptions;
                // #pragma omp parallel for shared (newPomo, otherPomo, zHelper) num_threads(omp_get_num_procs()*2)
                // fprintf(stderr, "Join interf with: ");
                // printPOMO(otherPomo);
                for (auto varItr: curPomo) {
                    // PartialOrderWrapper tmpPO = PartialOrderWrapper();
                    auto searchOtherPomo = otherPomo.find(varItr.first);
                    // don't need this search again
                    // if (searchInterfPomo == interfpomo.end()) {
                    //     fprintf(stderr, "ERROR: Variable mismatch in POMOs\n");
                    //     exit(0);
                    // }
                    
                    // join the two partial orders
                    // tmpPO.copy(varItr.second);
                    // fprintf (stderr, "Joining:%s and %s\n", varItr.second.toString().c_str(), searchOtherPomo->second.toString().c_str());
                    PartialOrder tmpPO = PartialOrderWrapper::join(varItr.second, searchOtherPomo->second);
                    // fprintf(stderr, "POMO after join: %s\n", tmpPO->toString().c_str());
                    // check what to do for each variable
                    getVarOption(&varoptions, varItr.first, tmpPO, searchOtherPomo->second);

                    newPomo.emplace(varItr.first, tmpPO);
                    // fprintf(stderr, "Pomo so far:\n");
                    // newPomo.printPOMO();
                }

                // create new ApDomain for this POMO
                // print varoptions
                // for (auto it=varoptions.begin(); it!=varoptions.end(); it++) {
                //     fprintf(stderr, "%s: ",it->first.c_str());
                //     switch (it->second) {
                //         case UNKNOWN:
                //             fprintf(stderr, "Unknown");
                //             break;
                //         case DONOTHING:
                //             fprintf(stderr, "Do nothing");
                //             break;
                //         case MERGE:
                //             fprintf(stderr, "Merge");
                //             break;
                //         case COPY:
                //             fprintf(stderr, "Copy");
                //             break;
                //         default:
                //             fprintf(stderr, "Default");
                //             break;
                //     }
                //     fprintf(stderr, "\n");
                // }
                tmpDomain.applyInterference("", otherItr.second, true, &varoptions);
            }
            auto searchNewPomo = newenvironment.find(newPomo);
            if (searchNewPomo == newenvironment.end())
                newenvironment[newPomo] = tmpDomain;
            else
                searchNewPomo->second.joinApDomain(tmpDomain);
        }
        // newenvironment[newPomo] = tmpDomain;
    }
    environment = newenvironment;
    if (other.isModified() && !isModified()) setModified();
    // for (auto it=other.begin(); it!=other.end(); ++it) {
    //     POMO pomo = it->first;
    //     ApDomain newDomain;
    //     newDomain.copyApDomain(it->second);

    //     // if the pomo already exist in the current enviornment,
    //     // join it with the existing one
    //     // else add it to the current environment
    //     auto searchPomo = environment.find(pomo);
    //     if (searchPomo != environment.end()) {
    //         newDomain.joinOnVars(searchPomo->second, globalVars);
    //     }
    //     environment[pomo] = newDomain;
    // }
    // fprintf(stderr, "Env after joinOnVars:\n");
    // printEnvironment();
}

void EnvironmentPOMO::copyOnVars(EnvironmentPOMO &other, vector<string> &vars) {
    if (environment.size() > 1) {
        fprintf(stderr, "ERROR: Please create new function for each thread create call\n");
        exit(0);
    }
    unordered_map<POMO, ApDomain> newenv;
    ApDomain funcOldApDomain;
    funcOldApDomain.copyApDomain(environment.begin()->second);
    for (auto it: other) {
        ApDomain tmpApDomain;
        tmpApDomain.copyApDomain(funcOldApDomain);
        tmpApDomain.copyOnVars(it.second, vars);
        newenv[it.first] = tmpApDomain;
    }
    environment = newenv;
    if (other.isModified() && !isModified()) setModified();
}

void EnvironmentPOMO::applyInterference(
    string &interfVar, 
    EnvironmentPOMO &interfEnv, 
    InstNum &curInst,
    InstNum &interfInst
) {
    // fprintf(stderr, "Env before applying interf:\n");
    // printEnvironment();

    unordered_map <POMO, ApDomain> newenvironment;
    for (auto curIt:environment) {
        for (auto interfIt:interfEnv) {
            POMO curPomo = curIt.first;
            POMO interfpomo = interfIt.first;
            
            // check if POMO are conssistent for all variables
            bool apply = true;
            for (auto varIt:curPomo) {
                auto searchInterfPomo = interfpomo.find(varIt.first);
                if (searchInterfPomo == interfpomo.end()) {
                    fprintf(stderr, "ERROR: Variable %s mismatch in POMOs in applyinterf\n", varIt.first.c_str());
                    exit(0);
                }
                if (!varIt.second.isConsistent(searchInterfPomo->second)) {
					// fprintf(stderr, "not consistent\n");
                    apply=false;
                    break;
                }
                // check domain-level feasibility 
                if (!varIt.second.isFeasible(searchInterfPomo->second, interfInst, curInst)) {
					// fprintf(stderr, "not feasible\n");
                    apply=false;
                    break;
                }
                if (!varIt.second.isConsistentRMW(searchInterfPomo->second)) {
					// fprintf(stderr, "not consistent rmw\n");
                    apply=false;
                    break;
                }
            }

            if (apply) {
                // fprintf(stderr, "Join interf with: ");
                // printPOMO(interfpomo);
                
                // merge the two partial orders
                POMO newPomo;
                // passed to applyinterf of ApDomain to make sure the copy/merging of 
                // ApDomain is consistent with POMO. Map from Variable --> options
                map<string, options> varoptions;

                // #pragma omp parallel for shared (newPomo, interfpomo, zHelper) num_threads(omp_get_num_procs()*2)
                for (auto varIt: curPomo) {
                    // PartialOrderWrapper tmpPO = PartialOrderWrapper(PartialOrder());
                    auto searchInterfPomo = interfpomo.find(varIt.first);
                    // don't need this search again
                    // if (searchInterfPomo == interfpomo.end()) {
                    //     fprintf(stderr, "ERROR: Variable mismatch in POMOs\n");
                    //     exit(0);
                    // }
                    
                    // join the two partial orders
                    // tmpPO.copy(varIt.second);
                    // fprintf (stderr, "Joining:%s and %s\n", varIt.second.toString().c_str(), searchInterfPomo->second.toString().c_str());
                    PartialOrder tmpPO = PartialOrderWrapper::join(varIt.second, searchInterfPomo->second);
                    // fprintf(stderr, "POMO after join: %s\n", tmpPO.toString().c_str());
                    
                    // for interfVar, add the store intruction in the end
                    if (varIt.first == interfVar) {   
                        PartialOrderWrapper::append(tmpPO, interfInst);
                    }

                    // check what to do for each variable
                    getVarOption(&varoptions, varIt.first, tmpPO, searchInterfPomo->second);

                    newPomo.emplace(varIt.first, tmpPO);
                    // fprintf(stderr, "Pomo so far:\n");
                    // printPOMO(newPomo);

                }

                // create new ApDomain for this POMO
                ApDomain tmpDomain;
                tmpDomain.copyApDomain(curIt.second);
                tmpDomain.applyInterference(interfVar, interfIt.second, true, &varoptions);
                if (!tmpDomain.isUnreachable()) {
                	auto searchPomo = newenvironment.find(newPomo);
                	if (searchPomo != newenvironment.end()) {
                	    tmpDomain.joinApDomain(searchPomo->second);
                	}
					newenvironment[newPomo] = tmpDomain;
					if (!isModified()) setModified();
			    }
			}
			else {
				newenvironment.insert(curIt); 
                auto searchPomo = newenvironment.find(curIt.first);
                if (searchPomo != newenvironment.end()) {
                	curIt.second.joinApDomain(searchPomo->second);
               	}
               	else {
					newenvironment.insert(curIt); 
               	}
			}
        }
    }
    environment = newenvironment;
    // fprintf(stderr, "env after apply interf\n");
    // printEnvironment();
}

void EnvironmentPOMO::joinEnvironment(EnvironmentPOMO &other) {
    for (auto it=other.begin(); it!=other.end(); ++it) {
        POMO pomo = it->first;
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);

        // if the pomo already exist in the current enviornment,
        // join it with the existing one
        // else add it to the current environment
        auto searchPomo = environment.find(pomo);
        if (searchPomo != environment.end()) {
            newDomain.joinApDomain(searchPomo->second);
        }
        else if (!newDomain.isUnreachable()) environment[pomo] = newDomain;
        if (other.isModified() && !isModified()) setModified();
    }
}

// Used for logical intructions
void EnvironmentPOMO::meetEnvironment(EnvironmentPOMO &other) {
    unordered_map <POMO, ApDomain> newenvironment;
    
    for (auto curIt=environment.begin(); curIt!=environment.end(); curIt++) {
        for (auto otherIt=other.begin(); otherIt!=other.end(); otherIt++) {
            // meet of ApDomain
            ApDomain newDomain;
            newDomain.copyApDomain(curIt->second);
            newDomain.meetApDomain(otherIt->second);

            if (curIt->first.lessThan(otherIt->first)) {
                auto searchPomo = newenvironment.find(curIt->first);
                if (searchPomo != newenvironment.end()) {
                    newDomain.joinApDomain(searchPomo->second);
                }
                else if(!newDomain.isUnreachable())
                newenvironment[curIt->first] = newDomain;
            }
            else if (otherIt->first.lessThan(curIt->first)) {
                auto searchPomo = newenvironment.find(otherIt->first);
                if (searchPomo != newenvironment.end()) {
                    newDomain.joinApDomain(searchPomo->second);
                }
                else if(!newDomain.isUnreachable())
                newenvironment[otherIt->first] = newDomain;
            }
            else {
                POMO newPomo;
                meetPOMO(curIt->first, otherIt->first, newPomo);
                // if curPomo alread exist join the newDomain with existing one
                auto searchPomo = newenvironment.find(newPomo);
                if (searchPomo != newenvironment.end()) {
                    newDomain.joinApDomain(searchPomo->second);
                }
                else if(!newDomain.isUnreachable())
                    newenvironment[newPomo] = newDomain;
                }
        }
    }
    environment = newenvironment;
    if (other.isModified() && !isModified()) setModified();
    // fprintf(stderr, "Env after meet:\n");
    // printEnvironment();
}

void EnvironmentPOMO::setVar(string &strVar) {
    for (auto it=environment.begin(); it!=environment.end();) {
        if (it->second.isUnreachable()) {
            it = environment.erase(it);
        }
        else {
            it->second.setVar(strVar);
            it++;
        }
    }
}

void EnvironmentPOMO::unsetVar(string &strVar) {
   for (auto it=environment.begin(); it!=environment.end();) {
        if (it->second.isUnreachable()) {
            it = environment.erase(it);
        }
        else {
            it->second.unsetVar(strVar);
            it++;
        }
    }
}

bool EnvironmentPOMO::isUnreachable() {
    bool isUnreach = true;
    for (auto it=environment.begin(); it!=environment.end();) {
        if (!it->second.isUnreachable()) {
            isUnreach = false;
            break;
        }
        else {it = environment.erase(it);}
    }
    return isUnreach;
}

void EnvironmentPOMO::getVarOption (map<string, options> *varoptions, 
    const string &varName,
    PartialOrder &curPartialOrder,
    const PartialOrder &interfPartialOrder
) {
    unordered_set<InstNum> lastsCurPO;
    curPartialOrder.getLasts(lastsCurPO);
    unordered_set<InstNum> lastsInterfPO;
    interfPartialOrder.getLasts(lastsInterfPO);

    options opt=UNKNOWN;
    
    if (lastsInterfPO.empty()) {
        // since there is no older write in interferring thread that can change the state, do nothing
        // fprintf(stderr, "%s: empty,do nothing\n", varName.c_str());
        opt = DONOTHING;
    }
    else if (lastsCurPO == lastsInterfPO) {
        // since no older write in current thread, the variable must be uninitialized, copy
        // fprintf(stderr, "%s: empty,copy\n", varName.c_str());
        opt = COPY;
    }
    else {   
        for (auto curItr: lastsCurPO) {
            for (auto interfItr: lastsInterfPO) {
                if (curPartialOrder.isOrderedBefore(interfItr, curItr) && (opt == UNKNOWN || opt == DONOTHING)) {
                    // last write of interferring thread is ordered before last write of current thread, do nothing
                    // fprintf(stderr, "%s: sb, do nothing\n", varName.c_str());
                    opt = DONOTHING;
                }
                else if (curPartialOrder.isOrderedBefore(curItr, interfItr) && (opt == UNKNOWN || opt == COPY)) {
                    // last write of current thread is ordered before last write of interferring thread, copy
                    // fprintf(stderr, "%s: sb, copy\n", varName.c_str());
                    opt = COPY;
                } 
                else {
                    // last writes of current thread and interferring threads are unordered, merge
                    // fprintf(stderr, "%s: no sb, merge\n", varName.c_str());
                    opt = MERGE;
                    break;
                }
            }
            if (opt == MERGE) break;
        }
    }
    varoptions->emplace(make_pair(varName, opt));
}

void EnvironmentPOMO::joinPOMO (const POMO &pomo1, const POMO &pomo2, POMO &joinedPOMO){
    // fprintf(stderr, "joining:\n");
    // printPOMO(pomo1);
    // printPOMO(pomo2);
    joinedPOMO = pomo1;
    if (pomo2.empty())
        return;
    for (auto it:pomo1) {
        auto searchPomo2 = pomo2.find(it.first);
        if (searchPomo2 == pomo2.end()) {
            fprintf(stderr, "ERROR: Variable %s mismatch in POMOs in joinPOMO\n", it.first.c_str());
            exit(0);
        }
        // join the two partial orders
        PartialOrder tmpPO = PartialOrderWrapper::join(it.second, searchPomo2->second);
        joinedPOMO.emplace(it.first, tmpPO);
    }
}

void EnvironmentPOMO::meetPOMO (const POMO &pomo1, const POMO &pomo2, POMO &meetPOMO){
    // fprintf(stderr, "joining:\n");
    // printPOMO(pomo1);
    // printPOMO(pomo2);
    meetPOMO = pomo1;
    if (pomo2.empty())
        return;
    for (auto it:pomo1) {
        auto searchPomo2 = pomo2.find(it.first);
        if (searchPomo2 == pomo2.end()) {
            fprintf(stderr, "ERROR: Variable %s mismatch in POMOs in meetPOMO\n", it.first.c_str());
            exit(0);
        }
        // join the two partial orders
        PartialOrder tmpPO = PartialOrderWrapper::meet(it.second, searchPomo2->second);
        meetPOMO.emplace(it.first, tmpPO);
    }
}

void EnvironmentPOMO::printEnvironment() {
    fprintf(stderr, "\n--Environment--\n");
    fprintf(stderr, "Modified: %d\n", isModified());
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        POMO pomo = it->first;
        fprintf (stderr, "Modification Order:\n");
        pomo.printPOMO();
        // printPOMO(pomo);
        // fprintf(stderr, "printing ApDomain\n");
        it->second.printApDomain();
        fprintf(stderr, "\n");
    }
}

// void EnvironmentPOMO::printPOMO(const POMO &pomo) {
//     // fprintf(stderr, "Printing POMO\n");
//     for (auto it=pomo.begin(); it!=pomo.end(); ++it) {
//         fprintf(stderr, "%s: ", it->first.c_str());
//         // if (it->second)
//             fprintf(stderr, "%s\n", it->second.toString().c_str());
//         // else fprintf(stderr, "NULL");
//         // fprintf(stderr, "\n");
//     }
//     // fprintf(stderr, "printing done\n");
// }

unordered_map<POMO, ApDomain>::iterator EnvironmentPOMO::begin() {
	return environment.begin();
}

unordered_map<POMO, ApDomain>::iterator EnvironmentPOMO::end() {
	return environment.end();
}



