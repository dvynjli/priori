#include "domain.h"


bool ApDomain::operator== (const ApDomain &other) const {
    ap_abstract1_t tempAbsVal = other.absValue;
    return ap_environment_is_eq(env, other.env) && 
        ap_abstract1_is_eq(man, &absValue, &tempAbsVal);
}

// bool ApDomain::operator!= (ApDomain other) {
//     return !(operator==(other));
// }

void ApDomain::init(vector<string> globalVars, vector<string> functionVars){
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

void ApDomain::initHasChanged(vector<string> globalVars) {
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it) {
        hasChanged[(*it)] = false;
    }
}

void ApDomain::setHasChanged(string var) {
    auto searchHasChanged = hasChanged.find(var);
    if (searchHasChanged != hasChanged.end() && !searchHasChanged->second) {
        hasChanged[var] = true;
    }
}

void ApDomain::assignZerosToGlobals(vector<string> globalVars) {
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
        fprintf(stderr, "unkown domain\n");
        exit(0);
    }
}

ap_environment_t* ApDomain::initEnvironment(vector<string> globalVars, vector<string> functionVars){
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

void ApDomain::copyApDomain(ApDomain copyFrom) {
    man = copyFrom.man;
    env = ap_environment_copy(copyFrom.env);
    absValue = ap_abstract1_copy(man, &copyFrom.absValue);
    // relHead = copyFrom.relHead;
    hasChanged = copyFrom.hasChanged;
}

void ApDomain::joinApDomain(ApDomain other) {
    ap_abstract1_join(man, true, &absValue, &other.absValue);
}

void ApDomain::meetApDomain(ApDomain other){
    ap_abstract1_meet(man, true, &absValue, &other.absValue);
}

void ApDomain::addVariable(string varName) {
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

void ApDomain::performUnaryOp(operation oper, string strTo, int intOp) {
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

void ApDomain::performUnaryOp(operation oper, string strTo, string strOp) {
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

void ApDomain::performBinaryOp(operation oper, string strTo, string strOp1, string strOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, 1, strOp2, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, 1, strOp2, AP_END);
            break;
        case MUL:
            // for multiplication of two variables, need to intervalize one
            ap_interval_t *fromInterval = ap_abstract1_bound_variable(man, &absValue, (ap_var_t)strOp2.c_str());
            ap_linexpr1_set_list(&expr, AP_COEFF_I, fromInterval, strOp1.c_str(), AP_END);
            break;
        default:
            fprintf(stderr, "WARNING: unsupported binary operation\n");
            break;
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
}

void ApDomain::performBinaryOp(operation oper, string strTo, string strOp1, int intOp2) {
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
}

void ApDomain::performBinaryOp(operation oper, string strTo, int intOp1, string strOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 2);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp2, AP_CST_S_INT, intOp1, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, -1, strOp2, AP_CST_S_INT, intOp1, AP_END);
            break;
        case MUL:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, intOp1, strOp2, AP_END);
            break;
        default:
            fprintf(stderr, "WARNING: unsupported binary operation\n");
            break;
    }
    // ap_linexpr1_fprint(stderr, &expr);
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    setHasChanged(strTo);
}

void ApDomain::performBinaryOp(operation oper, string strTo, int intOp1, int intOp2) {
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
}

void ApDomain::performCmpOp(operation oper, string strOp1, int intOp2) {
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

void ApDomain::performCmpOp(operation oper, int intOp1, string strOp2) {
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

void ApDomain::performCmpOp(operation oper, int intOp1, int intOp2) {
    // never occur
    fprintf(stderr, "performCmpOp() with both operand of condition as constant. This function should never called!!");
    exit(0);
}

void ApDomain::performCmpOp(operation oper, string strOp1, string strOp2) {
    fprintf(stderr, "performCmpOp() with both operand of condition as constant. This function is not implemented yet!!");
    exit(0);
}

void ApDomain::printApDomain() {
    ap_abstract1_fprint(stderr, man,  &absValue);
}

void ApDomain::applyInterference(string interfVar, ApDomain fromApDomain, bool isRelAcqSync) {
    // If this is a part of release-acquire sequence, copy the values of all the global vars,
    // else only the variable for which interference is 
    ap_var_t apInterVar;
    if (isRelAcqSync) {
        // fprintf(stderr, "applyinterf in ApDom for var %s. Dom before apply:\n", interfVar.c_str());
        // printApDomain();

        for (auto it=hasChanged.begin(); it!=hasChanged.end(); ++it) {
            apInterVar = (ap_var_t) it->first.c_str();
            if (ap_environment_dim_of_var(env, apInterVar) == AP_DIM_MAX) {
                fprintf(stderr, "ERROR: Interfering variable not in domain. Something went wrong.\n");
                exit(0);
            }
            if (it->second && it->first!=interfVar) {
                // there has been some event that has initialized the variable. Need to join the environment of this variable
                ap_abstract1_t tmpValue = ap_abstract1_copy(man, &absValue);
                
                // initialize it with the value of variable to be joined
                ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apInterVar);
                ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
                ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
                tmpValue = ap_abstract1_assign_linexpr(man, true, &tmpValue, apInterVar, &expr, NULL);
                
                // join the two abstract values
                absValue =  ap_abstract1_join(man, true, &tmpValue, &absValue);
            }
            else {
                // the variable is unintialized. Copy from the fromApDomain
                ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apInterVar);
                ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
                ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
                absValue = ap_abstract1_assign_linexpr(man, true, &absValue, apInterVar, &expr, NULL);
                setHasChanged(it->first);
            }
        }
        // fprintf(stderr, "dom after apply:\n");
        // printApDomain();
    }

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
        ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromApDomain.man, &fromApDomain.absValue, apInterVar);
        // ap_interval_fprint(stderr, fromInterval);
        ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
        ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
        absValue = ap_abstract1_assign_linexpr(man, true, &absValue, apInterVar, &expr, NULL);
        setHasChanged(interfVar);
    }

    // fprintf(stderr, "ApDomain after interf:\n");
    // printApDomain();
}

void ApDomain::setVar(string strVar) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_S_INT, 1, AP_END);
    ap_var_t var = (ap_var_t) strVar.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void ApDomain::unsetVar(string strVar) {
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
    }
}

void ApDomain::performNECmp(string strOp1, int intOp2) {
    // ltDomain = strOp1 < intOp2
    ApDomain ltDomain;
    ltDomain.copyApDomain(*this);
    ltDomain.performCmpOp(LT, strOp1, intOp2);
    // this = strOp1 > intOp1
    performCmpOp(GT, strOp1, intOp2);
    // this = ltDomain join this
    joinApDomain(ltDomain);
}

void ApDomain::performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t absValue) {
    /* assign x = 1 */
    fprintf(stderr, "Assigning x = 1\n");
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 1);
    ap_linexpr1_set_list(&expr, AP_CST_S_INT, 1, AP_END);
    ap_linexpr1_fprint(stderr, &expr);
    ap_var_t var = ap_environment_var_of_dim(env, 0);
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    fprintf(stderr, "assigned linexpr to x\n");
    ap_abstract1_fprint(stderr, man, &absValue);
    // ap_linexpr1_clear(&expr);

    /* assign y = x + 10 */
    fprintf(stderr, "Assigning y = x + 10\n");
    ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, "x", AP_CST_S_INT, 10, AP_END);
    ap_linexpr1_fprint(stderr, &expr);
    var = ap_environment_var_of_dim(env, 1);
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    fprintf(stderr, "assigned linexpr to x\n");
    ap_abstract1_fprint(stderr, man, &absValue);
    // ap_linexpr1_clear(&expr);

    /* assign x = x + y */
    fprintf(stderr, "Assigning x = x + y\n");
    ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, "x", AP_COEFF_S_INT, 1, "y", AP_CST_S_INT, 0, AP_END);
    ap_linexpr1_fprint(stderr, &expr);
    var = ap_environment_var_of_dim(env, 0);
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    fprintf(stderr, "assigned linexpr to x\n");
    ap_abstract1_fprint(stderr, man, &absValue);
    
    ap_linexpr1_clear(&expr);

}



//////////////////////////////////////
//      class EnvironmentRelHead     
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
        relHead[(*it)] = nullptr;
    }
    return relHead;
}

// llvm::Instruction* EnvironmentRelHead::getRelHead(string var) {
//     return relHead[var];
// }

void EnvironmentRelHead::addRelHead(string var, llvm::Instruction *head) {
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

void EnvironmentRelHead::changeRelHeadIfNull(string var, llvm::Instruction *head) {
    map <REL_HEAD, ApDomain> newEnvironment;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        if (relHead[var] == nullptr)
            relHead[var] = head;
        newEnvironment[relHead] = it->second;
    }
    environment = newEnvironment;
}

void EnvironmentRelHead::changeRelHead(string var, llvm::Instruction *head) {
    map <REL_HEAD, ApDomain> newEnvironment;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        relHead[var] = head;
        newEnvironment[relHead] = it->second;
    }
    environment = newEnvironment;
}

void EnvironmentRelHead::changeRelHeadToNull(string var, llvm::Instruction *inst) {
    map <REL_HEAD, ApDomain> newEnvironment;
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        REL_HEAD relHead(it->first);
        // RelSequence terminated when a relaxed write from different thread is occured
        // relHead is changed to null only if existing relHead is from different thread
        if (relHead[var] != nullptr && inst->getFunction() != relHead[var]->getFunction())
            relHead[var] = nullptr;
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

void EnvironmentRelHead::applyInterference(
    string interfVar, 
    EnvironmentRelHead fromEnv, 
    bool isRelAcqSync, 
    Z3Minimal &zHelper, 
    llvm::Instruction *interfInst=nullptr, 
    llvm::Instruction *curInst=nullptr
) {
    // fprintf(stderr, "Env before applying interf:\n");
    // printEnvironment();

    if (isRelAcqSync) {
        carryEnvironment(interfVar, fromEnv);
    }
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
                tmpDomain.applyInterference(interfVar, interfItr->second, isRelAcqSync);
                curDomain.joinApDomain(tmpDomain);
            }
            environment[curRelHead] = curDomain;
        }
    }
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
                newDomain.applyInterference(interfVar, interfItr->second, true);
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
        if (it->second != nullptr)
            it->second->print(llvm::errs());
        else fprintf(stderr, "NULL");
        fprintf(stderr, "\n");
    }
}



//////////////////////////////////////
//      class EnvironmentPOMO         //
//////////////////////////////////////
bool EnvironmentPOMO::operator== (const EnvironmentPOMO &other) const {
    return environment==other.environment;
}


void EnvironmentPOMO::init(vector<string> globalVars, vector<string> functionVars){
    POMO pomo = initPOMO(globalVars);
    ApDomain dom;
    dom.init(globalVars, functionVars);
    // fprintf(stderr, "dom done. assign to env\n");
    environment[pomo] = dom;
    // printEnvironment();
}

POMO EnvironmentPOMO::initPOMO(vector<string> globalVars){
    POMO pomo;
    for (auto it=globalVars.begin(); it!=globalVars.end(); ++it) {
        pomo[(*it)] = new PartialOrder();
    }
    return pomo;
}

void EnvironmentPOMO::copyEnvironment(EnvironmentPOMO copyFrom){
    // environment = copyFrom.environment;
    environment.clear();
    for (auto it=copyFrom.environment.begin(); it!=copyFrom.environment.end(); ++it) {
        ApDomain newDomain;
        newDomain.copyApDomain(it->second);
        environment[it->first]=newDomain;
    }
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

void EnvironmentPOMO::performBinaryOp(operation oper, string strTo, string strOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, strOp1, intOp2);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string strTo, int intOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, intOp1, strOp2);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string strTo, int intOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, intOp1, intOp2);
    }
}

void EnvironmentPOMO::performBinaryOp(operation oper, string strTo, string strOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performBinaryOp(oper, strTo, strOp1, strOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, string strOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, strOp1, intOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, int intOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, intOp1, strOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, int intOp1, int intOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, intOp1, intOp2);
    }
}

void EnvironmentPOMO::performCmpOp(operation oper, string strOp1, string strOp2) {
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        it->second.performCmpOp(oper, strOp1, strOp2);
    }
}

void EnvironmentPOMO::applyInterference(
    string interfVar, 
    EnvironmentPOMO fromEnv, 
    bool isRelAcqSync, 
    Z3Minimal &zHelper, 
    llvm::Instruction *interfInst=nullptr,
    llvm::Instruction *curInst=nullptr
) {
    fprintf(stderr, "Env before applying interf:\n");
    printEnvironment();

    // We are assuming RA. Hence everything is RelAcqSync
    if (isRelAcqSync) {
        for (auto curIt:environment) {
            for (auto interfIt:fromEnv) {
                POMO curPomo = curIt.first;
                POMO interfpomo = interfIt.first;
                
                // check if POMO are conssistent for all variables
                bool apply = true;
                for (auto varIt:curPomo) {
                    auto searchInterfPomo = interfpomo.find(varIt.first);
                    if (searchInterfPomo == interfpomo.end()) {
                        fprintf(stderr, "ERROR: Variable mismatch in POMOs");
                        exit(0);
                    }
                    if (!varIt.second->isConsistent(*(searchInterfPomo->second))) {
                        apply=false;
                        break;
                    }
                    // check domain-level feasibility 
                    if (!varIt.second->isFeasible(zHelper, *(searchInterfPomo->second), interfInst, curInst)) {
                        apply=false;
                        break;
                    } 
                }

                if (apply) {
                    // merge the two partial orders
                    PartialOrder *tmpPO = new PartialOrder();
                    for (auto varIt: curPomo) {
                        auto searchInterfPomo = interfpomo.find(varIt.first);
                        // don't need this search again
                        // if (searchInterfPomo == interfpomo.end()) {
                        //     fprintf(stderr, "ERROR: Variable mismatch in POMOs");
                        //     exit(0);
                        // }
                        
                        // join the two partial orders
                        tmpPO->copy(*(varIt.second));
                        fprintf (stderr, "Joining:%s and %s\n", tmpPO->toString().c_str(), searchInterfPomo->second->toString().c_str());

                        tmpPO->join(zHelper, *(searchInterfPomo->second));

                        fprintf(stderr, "POMO after join: %s\n", tmpPO->toString().c_str());
                        
                        // for interfVar, add the store intruction in the end
                        if (varIt.first == interfVar) {   
                            tmpPO->append(zHelper, interfInst);
                        }
                        curPomo[varIt.first] = tmpPO;
                        fprintf(stderr, "Pomo so far:\n");
                        printPOMO(curPomo);
                    }
                    // create new ApDomain for this POMO
                    ApDomain tmpDomain;
                    tmpDomain.copyApDomain(curIt.second);
                    tmpDomain.applyInterference(interfVar, interfIt.second, isRelAcqSync);
                    environment[curIt.first] = tmpDomain;
                }
            }
        }
    }
    else {
        // Need to fill this to add supposrt for models other than RA
        /* for (auto it=environment.begin(); it!=environment.end(); ++it) {
            POMO curPomo = it->first;
            
            bool apply = true;
            for (auto pomoIt=curPomo.begin(); pomoIt!=curPomo.end(); ++pomoIt) {
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
                tmpDomain.applyInterference(interfVar, interfItr->second, isRelAcqSync);
                curDomain.joinApDomain(tmpDomain);
            }
            environment[curRelHead] = curDomain;
        } */
    }

    fprintf(stderr, "env after apply interf\n");
    printEnvironment();
}

void EnvironmentPOMO::carryEnvironment(string interfVar, EnvironmentPOMO fromEnv) {
    /* map <REL_HEAD, ApDomain> newEnvironment;
        for (auto interfItr=fromEnv.environment.begin(); interfItr!=fromEnv.environment.end(); ++interfItr) {
            for (auto curItr=environment.begin(); curItr!=environment.end(); ++curItr) {
                REL_HEAD curRelHead(curItr->first);
                REL_HEAD interfRelHead(interfItr->first);
                curRelHead[interfVar] = interfRelHead[interfVar];
                ApDomain newDomain;
                newDomain.copyApDomain(curItr->second);
                newDomain.applyInterference(interfVar, interfItr->second, true);
                auto searchRelHead = environment.find(curRelHead);
                // if (searchRelHead != environment.end()) {
                //     newDomain.joinApDomain(searchRelHead->second);
                // }
                newEnvironment[curRelHead] = newDomain;
            }
        }
        environment = newEnvironment; */
}

void EnvironmentPOMO::joinEnvironment(EnvironmentPOMO other) {
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
        environment[pomo] = newDomain;
    }
}
// Used for logical intructions
void EnvironmentPOMO::meetEnvironment(Z3Minimal &zHelper, EnvironmentPOMO other) {
    map <POMO, ApDomain> newenvironment;

    for (auto curIt: environment) {
        for (auto otherIt: other) {
            // join the POMOs
            POMO curPomo;
            joinPOMO(zHelper, curIt.first, otherIt.first, curPomo);
            // fprintf(stderr, "curPomo:\n");
            // printPOMO(curPomo);

            // meet of ApDomain
            ApDomain newDomain;
            newDomain.copyApDomain(curIt.second);
            newDomain.meetApDomain(otherIt.second);

            // if curPomo alread exist join the newDomain with existing one
            auto searchPomo = environment.find(curPomo);
            if (searchPomo != environment.end()) {
                newDomain.joinApDomain(searchPomo->second);
            }
            newenvironment[curPomo] = newDomain;
        }
    }
    environment = newenvironment;
    // fprintf(stderr, "Env after meet:\n");
    // printEnvironment();
}

void EnvironmentPOMO::appendInst(Z3Minimal &zHelper, llvm::StoreInst *storeInst, string var) {
    for (auto it: environment) {
        auto searchVarPomo = it.first.find(var);
        if (searchVarPomo == it.first.end()) {
            fprintf(stderr, "ERROR: Variable not found in POMOs");
            exit(0);
        }
        fprintf(stderr, "appending from POMO\n");
        searchVarPomo->second->append(zHelper, storeInst);
    }
}


bool EnvironmentPOMO::isUnreachable() {
    bool isUnreach = true;
    for (auto it:environment) {
        if (!it.second.isUnreachable()) {
            isUnreach = false;
            break;
        }
    }
    return isUnreach;
}

void EnvironmentPOMO::joinPOMO (Z3Minimal &zHelper, POMO pomo1, POMO pomo2, POMO joinedPOMO){
    joinedPOMO = pomo1;
    for (auto it:joinedPOMO) {
        auto searchPomo2 = pomo2.find(it.first);
        if (searchPomo2 == pomo2.end()) {
            fprintf(stderr, "ERROR: Variable mismatch in POMOs");
            exit(0);
        }
        // join the two partial orders
        it.second->join(zHelper, *(searchPomo2->second));
    }
}

void EnvironmentPOMO::printEnvironment() {
    fprintf(stderr, "\n--Environment--\n");
    for (auto it=environment.begin(); it!=environment.end(); ++it) {
        POMO pomo = it->first;
        fprintf (stderr, "Modification Order:\n");
        printPOMO(pomo);
        // fprintf(stderr, "printing ApDomain\n");
        it->second.printApDomain();
        fprintf(stderr, "\n");
    }
}

void EnvironmentPOMO::printPOMO(POMO pomo) {
    // fprintf(stderr, "Printing POMO\n");
    for (auto it=pomo.begin(); it!=pomo.end(); ++it) {
        fprintf(stderr, "%s: ", it->first.c_str());
        if (it->second)
            fprintf(stderr, "%s", it->second->toString().c_str());
        else fprintf(stderr, "NULL");
        fprintf(stderr, "\n");
    }
    // fprintf(stderr, "printing done\n");
}

map<POMO, ApDomain>::iterator EnvironmentPOMO::begin() {
	return environment.begin();
}

map<POMO, ApDomain>::iterator EnvironmentPOMO::end() {
	return environment.end();
}



