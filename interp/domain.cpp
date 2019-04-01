#include "domain.h"


bool Domain::operator== (const Domain &other) const {
    ap_abstract1_t tempAbsVal = other.absValue;
    return ap_environment_is_eq(env, other.env) && 
        ap_abstract1_is_eq(man, &absValue, &tempAbsVal);
}

// bool Domain::operator!= (Domain other) {
//     return !(operator==(other));
// }

void Domain::init(string domainType, vector<string> intVars){
    fprintf(stderr, "initializing ap_man\n");
    man = initApManager(domainType);
    DEBUG && fprintf(stderr, "Init Env\n");
    env = initEnvironment(intVars);
    ap_environment_fdump(stderr, env);
    // ap_var_t var = ap_environment_var_of_dim(env, 0);
    // fprintf(stderr, "var: %s\n", var);
    // DEBUG && fprintf(stderr, "creating top\n");
    absValue = ap_abstract1_top(man, env);
    assignZerosToAllVars();
    // printDomain();


    // DEBUG && fprintf(stderr, "performing transforms\n");
    // performTrasfer(man, env, absValue);
}

void Domain::assignZerosToAllVars() {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
    for(int i = 0; i< env->intdim; i++) {
        ap_linexpr1_set_list(&expr, AP_CST_S_INT, 0, AP_END);
        ap_var_t var = ap_environment_var_of_dim(env, i);
        absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
    }
}

ap_manager_t* Domain::initApManager(string domainType) {
    //TODO: parameterize by command line arg
    if (domainType.compare("box") == 0)
        return box_manager_alloc();
    else {
        fprintf(stderr, "unkown domain %s\n", domainType.c_str());
        exit(0);
    }
}

ap_environment_t* Domain::initEnvironment(vector<string> intVars){
    ap_var_t intAp[intVars.size()];
    int i = 0;
    for (auto it = intVars.begin(); it != intVars.end(); it++, i++){
        intAp[i] = (ap_var_t)(it->c_str());
    }
    //TODO: use if providing support for floats
    ap_var_t floatAp[0];
    
    //return nullptr;
    return ap_environment_alloc(intAp, intVars.size(), floatAp, 0);
}

void Domain::copyDomain(Domain copyFrom) {
    man = copyFrom.man;
    env = ap_environment_copy(copyFrom.env);
    absValue = ap_abstract1_copy(man, &copyFrom.absValue);
}

void Domain::joinDomain(Domain other) {
    ap_abstract1_join(man, true, &absValue, &other.absValue);
}

void Domain::addVariable(string varName) {
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

void Domain::performUnaryOp(operation oper, string strTo, int intOp) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
    switch(oper) {
        // case LOAD is not possible
        case STORE:
            ap_linexpr1_set_list(&expr, AP_CST_S_INT, intOp, AP_END);
            break;
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void Domain::performUnaryOp(operation oper, string strTo, string strOp) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
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
}

void Domain::performBinaryOp(operation oper, string strTo, string strOp1, string strOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, 1, strOp2, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_COEFF_S_INT, 1, strOp2, AP_END);
            break;
        case MUL:
            // TODO: can't do using linexpr
            break;
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void Domain::performBinaryOp(operation oper, string strTo, string strOp1, int intOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
    switch(oper) {
        case ADD:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_CST_S_INT, intOp2, AP_END);
            break;
        case SUB:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, 1, strOp1.c_str(), AP_CST_S_INT, intOp2, AP_END);
            break;
        case MUL:
            ap_linexpr1_set_list(&expr, AP_COEFF_S_INT, intOp2, strOp1.c_str(), AP_END);
            break;
    }
    // ap_linexpr1_fprint(stderr, &expr);
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void Domain::performBinaryOp(operation oper, string strTo, int intOp1, string strOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
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
    }
    // ap_linexpr1_fprint(stderr, &expr);
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void Domain::performBinaryOp(operation oper, string strTo, int intOp1, int intOp2) {
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
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
    }
    ap_var_t var = (ap_var_t) strTo.c_str();
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, var, &expr, NULL);
}

void Domain::performCmpOp(operation oper, string strTo, string strOp1, int intOp2) {
    // Constr ->expr->assign
}

void Domain::performCmpOp(operation oper, string strTo, int intOp1, string strOp2) {

}

void Domain::performCmpOp(operation oper, string strTo, int intOp1, int intOp2) {
    // never occur
    fprintf(stderr, "performCmpOp() with both operand of condition as constant. This function should never called!!");
    exit(0);
}

void Domain::performCmpOp(operation oper, string strTo, string strOp1, string strOp2) {

}
    

void Domain::printDomain() {
    ap_abstract1_fdump(stderr, man,  &absValue);
}

void Domain::applyInterference(string interfVar, Domain fromDomain) {
    ap_var_t apInterVar = (ap_var_t) interfVar.c_str();
    if (ap_environment_dim_of_var(env, apInterVar) == AP_DIM_MAX) {
        fprintf(stderr, "ERROR: Interfering variable not in domain. Something went wrong.\n");
        exit(0);
    }

    // fprintf(stderr, "Appling insterf from domain: \n");
    // fromDomain.printDomain();
    // fprintf(stderr, "To domain: \n");
    // printDomain();
    // fprintf(stderr, "On var %s\n", interfVar.c_str());

    ap_interval_t *fromInterval = ap_abstract1_bound_variable(fromDomain.man, &fromDomain.absValue, apInterVar);
    // ap_interval_fprint(stderr, fromInterval);
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
    ap_linexpr1_set_list(&expr, AP_CST_I, fromInterval, AP_END);
    absValue = ap_abstract1_assign_linexpr(man, true, &absValue, apInterVar, &expr, NULL);

    // fprintf(stderr, "Domain after interf:\n");
    // printDomain();
}

void Domain::performTrasfer(ap_manager_t *man, ap_environment_t *env, ap_abstract1_t absValue) {
    /* assign x = 1 */
    fprintf(stderr, "Assigning x = 1\n");
    ap_linexpr1_t expr = ap_linexpr1_make(env, AP_LINEXPR_SPARSE, 0);
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

