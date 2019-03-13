#include "domain.h"

void Domain::init(string domainType, vector<string> intVars){
    fprintf(stderr, "initializing ap_man\n");
    man = initApManager(domainType);
    DEBUG && fprintf(stderr, "Init Env\n");
    env = initEnvironment(intVars);
    ap_environment_fdump(stderr, env);
    // ap_var_t var = ap_environment_var_of_dim(env, 0);
    // fprintf(stderr, "var: %s\n", var);
    DEBUG && fprintf(stderr, "creating top\n");
    absValue = ap_abstract1_top(man, env);
    ap_abstract1_fdump(stderr, man,  &absValue);

    DEBUG && fprintf(stderr, "performing transforms\n");
    // performTrasfer(man, env, absValue);
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

void Domain::joinDomain(Domain other) {
    ap_abstract1_join(man, true, &absValue, &other.absValue);
}

void Domain::addVariable(string varName) {
    ap_var_t newIntAp = (ap_var_t)(varName.c_str());
    int oldSize = env->intdim;
    // env = ap_environment_add(env, &newIntAp, oldSize+1, NULL, 0);
}

void Domain::performUnaryOp(operation oper, string strTo, int intOp) {
    fprintf(stderr, "%d %d to %s\n", oper, intOp, strTo.c_str());
}
void Domain::performUnaryOp(operation oper, string strTo, string strOp) {
    fprintf(stderr, "%d %s to %s\n", oper, strOp.c_str(), strTo.c_str());
}

void performBinaryOp(operation oper, string strTo, string strOp1, string strOp2) {
    fprintf(stderr, "%d %s and %s to %s\n", oper, strOp1.c_str(), strOp2.c_str(), strTo.c_str());
}
void performBinaryOp(operation oper, string strTo, string strOp1, int intOp2) {
    fprintf(stderr, "%d %s and %d to %s\n", oper, strOp1.c_str(), intOp2, strTo.c_str());
}
void performBinaryOp(operation oper, string strTo, int intOp1, string strOp2) {
    fprintf(stderr, "%d %d and %s to %s\n", oper, intOp1, strOp2.c_str(), strTo.c_str());
}
void performBinaryOp(operation oper, string strTo, int intOp1, int intOp2) {
    fprintf(stderr, "%d %d and %d to %s\n", oper, intOp1, intOp2, strTo.c_str());
}
    

void Domain::printDomain() {
    ap_abstract1_fdump(stderr, man,  &absValue);
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
