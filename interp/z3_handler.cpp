#include "z3_handler.h"

void Z3Helper::initZ3(vector<string> globalVars) {
    // TODO: declare enum for mem order and variable (if possible).
    // and change function varOf and memOrder accordingly
    // might need to define <=  for acq and rel mem orders
    // can'r find how to use name of enum
    
    // const char * memOrderNames[] = { "rlx", "acq", "rel", "acq-rel", "seq_cst" };
    // z3::func_decl_vector memOrderEnumConsts(zcontext);
    // z3::func_decl_vector memOrderEnumTesters(zcontext);
    // z3::sort memOrderSort = zcontext.enumeration_sort("memOrder", 5, memOrderNames, memOrderEnumConsts, memOrderEnumTesters);

    // int noOfVars = globalVars.size();
    // const char * varNames[noOfVars];
    // for (int i=0; i<noOfVars; i++) {
    //     varNames[i] = globalVars[i].c_str();
    // }
    // z3::func_decl_vector varsEnumConsts(zcontext);
    // z3::func_decl_vector varsEnumTesters(zcontext);
    // z3::sort vars = zcontext.enumeration_sort("vars", noOfVars, varNames, varsEnumConsts, varsEnumTesters);
    try {
        z3::expr inst1 = zcontext.bv_const("inst1", BV_SIZE);
        z3::expr inst2 = zcontext.bv_const("inst2", BV_SIZE);
        z3::expr inst3 = zcontext.bv_const("inst3", BV_SIZE);
        z3::expr inst4 = zcontext.bv_const("inst4", BV_SIZE);
        z3::expr var1  = zcontext.bv_const("var1", BV_SIZE);
        z3::expr var2  = zcontext.bv_const("var2", BV_SIZE);
        z3::expr ord1  = zcontext.int_const("ord1");
        // z3::expr app = mhb(inst1, inst2) && mhb(inst2, inst3);

        // (s1,s2) in MHB && (s2,s3) in MHB => (s1,s3) in MHB
        z3::expr transitive_mhb = z3::forall(inst1, inst2, inst3, 
                z3::implies((mhb(inst1, inst2) && mhb(inst2, inst3)), 
                mhb(inst1, inst3) ));
        // (l,v) \in isLoad && (s1,v) \in isStore && (s2,v) \in isStore &&
        //  (s1,l) \in rf && (s1,s2) \in MHB
        // => (l,s2) \in MHB
        z3::expr fr = z3::forall(inst1, inst2, inst3, var1, 
                z3::implies(( isLoad(inst1) && isVarOf(inst1, var1) && 
                    isStore(inst2) && isVarOf(inst2, var1) &&
                    isStore(inst3) && isVarOf(inst3, var1) &&
                    rf(inst2, inst1) && mhb(inst2, inst3)), 
                mhb(inst1, inst3)));
        // z3::expr nrf = z3::forall(inst1, inst2, 
        //         z3::implies( (isLoad(inst1) && isStore(inst2) && mhb(inst1, inst2)), 
        //         not(rf(inst1, inst2))));
        
        Z3_fixedpoint_add_rule(zcontext, zfp, transitive_mhb, zcontext.str_symbol("Transitive-MHB"));
        Z3_fixedpoint_add_rule(zcontext, zfp, fr, zcontext.str_symbol("FR"));
        // Z3_fixedpoint_add_rule(zcontext, zfp, nrf, zcontext.str_symbol("nrf"));

        // ( (l,op) \in PO && l \in AcqOp) => (l,op) \in MCB
        z3::expr acqReordering = z3::forall(inst1, inst2, 
                z3::implies(po(inst1, inst2) && isLoad(inst1) && 
                    z3::exists(ord1, memOrderOf(inst1, ord1) && ord1>=ACQ),
                mcb(inst1, inst2)));
        Z3_fixedpoint_add_rule(zcontext, zfp, acqReordering, zcontext.str_symbol("Acq-Reordering"));
        // ( (op,s) \in PO && l \in RelOp) => (op,s) \in MCB
        z3::expr relReordering = z3::forall(inst1, inst2,
                z3::implies(po(inst1, inst2) && isStore(inst2) && 
                    z3::exists(ord1, memOrderOf(inst2, ord1) && ord1>=REL),
                mcb(inst1, inst2)));
        Z3_fixedpoint_add_rule(zcontext, zfp, relReordering, zcontext.str_symbol("Rel-Reordering"));

        // ( s \in RelOp && l \in AcqOp && 
        //   (l,v1) \in IsLoad && (s,v1) \in IsStore && 
        //   (s,l) \in RF &&
        //   (op1,s) \in MCB && (l,op2) \in MCB)
        // => (op1,op2) \in MCB
        z3::expr_vector xs(zcontext);
        xs.push_back(inst1);
        xs.push_back(inst2);
        xs.push_back(inst3);
        xs.push_back(inst4);
        xs.push_back(var1);
        z3::expr relAcqSeq = z3::forall(xs, 
                z3::implies(isStore(inst1) && isLoad(inst2) && 
                    z3::exists(ord1, memOrderOf(inst1, ord1) && ord1>=REL) &&
                    z3::exists(ord1, memOrderOf(inst2, ord1) && ord1>=ACQ) && 
                    isVarOf(inst1, var1) && isVarOf(inst2, var1) &&
                    rf(inst1, inst2) &&
                    mcb(inst3, inst1) && mcb(inst2, inst4),                            
                mcb(inst3, inst4)));
        Z3_fixedpoint_add_rule(zcontext, zfp, relAcqSeq, zcontext.str_symbol("Rel-Acq-Seq"));

        // cout << zfp.to_string() << "\n";

        // z3::expr a = zcontext.bv_val(2, BV_SIZE);
        // z3::expr b = zcontext.bv_val(5, BV_SIZE);
        // Z3_fixedpoint_add_rule(zcontext, zfp, isVarOf(a,b), zcontext.str_symbol("test"));

        // z3::expr query1 = isVarOf(a,a);
        // z3::expr query2 = isVarOf(a,b);
        // cout << zfp.query(query1) << "\n";
        // cout << zfp.query(query2) << "\n";
        
    } catch (z3::exception e) {cout << "Exception: " << e << "\n";}
}

void Z3Helper::addMHB (llvm::Instruction *from, llvm::Instruction *to) {
    const z3::expr fromExpr = getBitVec(from);
    const z3::expr toExpr = getBitVec(to);
    const z3::expr trueExpr = zcontext.bool_val(true);
    // unsigned int fromInt = (unsigned int) from;
    // unsigned int toInt = (unsigned int) to;
    // z3::expr fromExpr = zcontext.int_val(fromInt);
    // z3::expr toExpr = zcontext.int_val(toInt);
    // printf ("bitvec: %u, %u\t ints: %u, %u\t instr: %u, %u\n", fromExpr, toExpr, fromInt, toInt, from, to);
    // cout << "AddMHB:\n";
    // cout << "\tFrom: ";
    // from->print(llvm::outs());
    // cout << "\n\tTo: ";
    // to->print(llvm::outs());
    // cout << "\n";
    // cout << "bitvec from: " << fromExpr << ", to: " << toExpr << ", true: " << trueExpr << "\n";
    z3::expr app = mhb(fromExpr, toExpr);
    Z3_fixedpoint_add_rule(zcontext, zfp, app, NULL);
}

void Z3Helper::addLoadInstr (llvm::LoadInst *inst) {
    const z3::expr instExpr = getBitVec(inst);
    z3::expr app = isLoad(instExpr);
    Z3_fixedpoint_add_rule(zcontext, zfp, app, NULL);
    
    llvm::Value* fromVar = inst->getOperand(0);
    addInstToVar(instExpr, fromVar);

    llvm::AtomicOrdering ord = inst->getOrdering();
    addInstToMemOrd(instExpr, ord);
}

void Z3Helper::addStoreInstr (llvm::StoreInst *inst) {
    const z3::expr instExpr = getBitVec(inst);
    z3::expr app = isStore(instExpr);
    Z3_fixedpoint_add_rule(zcontext, zfp, app, NULL);
    
    llvm::Value* fromVar = inst->getPointerOperand();
    addInstToVar(instExpr, fromVar);

    llvm::AtomicOrdering ord = inst->getOrdering();
    addInstToMemOrd(instExpr, ord);
}

void Z3Helper::addInstToVar(z3::expr inst, llvm::Value *var) {
    const z3::expr varExpr = getBitVec(var);
    z3::expr app = isVarOf(inst, varExpr);
    Z3_fixedpoint_add_rule(zcontext, zfp, app, NULL);
}

void Z3Helper::addInstToMemOrd(z3::expr inst, llvm::AtomicOrdering ord) {
    const z3::expr ordExpr = getMemOrd(ord);
    z3::expr app = memOrderOf(inst, ordExpr);
    Z3_fixedpoint_add_rule(zcontext, zfp, app, NULL);
}

z3::expr Z3Helper::getBitVec (void *op) {
    unsigned int ptr = (unsigned int) op;
    return zcontext.bv_val(ptr, BV_SIZE);
}

z3::expr Z3Helper::getMemOrd(llvm::AtomicOrdering ord) {
    // cout << "-----Mem order: " << ord << " ----\n";
    enum mem_order ordInt;
    switch(ord) {
        case llvm::AtomicOrdering::NotAtomic:
            ordInt = NA;
            break;
        case llvm::AtomicOrdering::Monotonic:
            ordInt = RLX;
            break;
        case llvm::AtomicOrdering::Acquire:
            ordInt = ACQ;
            break;
        case llvm::AtomicOrdering::Release:
            ordInt = REL;
            break;
        case llvm::AtomicOrdering::AcquireRelease:
            ordInt = ACQ_REL;
            break;
        case llvm::AtomicOrdering::SequentiallyConsistent:
            ordInt = SEQ_CST;
            break;
        default:
            cout << "WARNING: Unknown mem order\n";
            break;
    }
    return zcontext.int_val(ordInt);
}

void Z3Helper::testFixedPoint() {
    // z3::sort s = zcontext.bv_sort(3);
    // z3::sort B = zcontext.bool_sort();
    // z3::func_decl edge = z3::function("edge", s, s, B);
    // z3::func_decl path = z3::function("path", s, s, B);
    // z3::expr a = zcontext.bv_const("a", 3);
    // z3::expr b = zcontext.bv_const("b", 3);
    // z3::expr c = zcontext.bv_const("c", 3);
    // z3::expr t = zcontext.bool_val(true);
    // z3::expr f = zcontext.bool_val(false);

    // try {
    //     z3::expr rule1 = z3::implies((edge(a,b)==t), (path(a,b)==t));
    //     Z3_fixedpoint_add_rule(zcontext, zfp, rule1, NULL);
    //     z3::expr rule2 = z3::implies( (path(a,b)==t && path(b,c)==t), path(a,c)==t );
    //     Z3_fixedpoint_add_rule(zcontext, zfp, rule2, NULL);

    //     z3::expr n1 = zcontext.bv_val(1,3);
    //     z3::expr n2 = zcontext.bv_val(2,3);
    //     z3::expr n3 = zcontext.bv_val(3,3);
    //     z3::expr n4 = zcontext.bv_val(4,3);

    //     Z3_fixedpoint_add_rule(zcontext, zfp, edge(n1,n2)==t, NULL);
    //     Z3_fixedpoint_add_rule(zcontext, zfp, edge(n1,n3)==t, NULL);
    //     Z3_fixedpoint_add_rule(zcontext, zfp, edge(n2,n4)==t, NULL);

    //     z3::expr q1 = zcontext.bool_const("q1");
    //     z3::expr q2 = zcontext.bool_const("q2");
    //     z3::expr q3 = zcontext.bv_const("q3", 3);
    //     Z3_fixedpoint_add_rule(zcontext, zfp, z3::implies(path(n1,n4)==t, q1), NULL);
    //     // Z3_fixedpoint_add_rule(zcontext, zfp, (path(n1,n4)==q1), NULL);

    //     cout << "\nFixed point: \n" << zfp.to_string() << "\n";
    //     Z3_lbool result = Z3_fixedpoint_query(zcontext, zfp, q1);
    //     if (result == Z3_L_UNDEF)
    //         cout << "undefined\n";
    //     else if (result == Z3_L_FALSE)
    //         cout << "unsat\n";
    //     else if (result == Z3_L_TRUE)
    //         cout << "sat\n";
    //     else cout << "something went wrong!!\n";

    // } catch (z3::exception e) { cout << "Exception: " << e << "\n";}


    z3::func_decl fun1 = z3::function ("fun1", zcontext.int_sort(), zcontext.bool_sort());
    z3::expr a = zcontext.int_val(5);
    // z3::expr t = zcontext.bool_val(true);
    // z3::expr f = zcontext.bool_val(false);
    // z3::expr app = (fun1(a)==t);
    // Z3_fixedpoint_add_rule(zcontext, zfp, app, NULL);
    // zsolver.add(app);
    // z3::expr x = zcontext.int_const("x");
    // z3::expr y = zcontext.int_const("y");
    // z3::expr invapp1 = z3::implies(not(fun1(x)), (fun1(x)));
    // z3::expr invapp2 = z3::implies((fun1(x)), not(fun1(x)));
    // zsolver.add(invapp1);
    // zsolver.add(invapp2);
    // Z3_fixedpoint_add_rule(zcontext, zfp, invapp1, NULL);
    // Z3_fixedpoint_add_rule(zcontext, zfp, invapp2, NULL);

    // cout << "zsolver: \n" << zsolver << "\n";
    // cout << "Z3 result: " << zsolver.check() << "\n";
    // cout << "Model: " << zsolver.get_model() << "\n";

    cout << "\nFixed point: \n" << zfp.to_string() << "\n";
    // z3::expr q1 = zcontext.bool_const("query1");
    // z3::expr test = z3::implies(zcontext.bool_val(true), q1);
    // Z3_fixedpoint_add_rule(zcontext, zfp, test, NULL);
    // cout << "query: " << test << "\n";
    Z3_lbool result = Z3_fixedpoint_query(zcontext, zfp, fun1(a));
    if (result == Z3_L_UNDEF)
        cout << "undefined\n";
    else if (result == Z3_L_FALSE)
        cout << "unsat\n";
    else if (result == Z3_L_TRUE)
        cout << "sat\n";
    else cout << "something went wrong!!\n";
    try {
        // cout << "Z3 fp query: " << zfp.query(x) << "\n";
    } catch (z3::exception e) {cout << "Exception: " << e << "\n";}
}

/* void Z3Helper::test_bv_fun() {
    z3::expr int2Expr = zcontext.bv_val(1024, 2);
    z3::expr int4Expr = zcontext.bv_val(1024, 4);
    z3::expr int8Expr = zcontext.bv_val(1024, 8);
    z3::expr int16Expr = zcontext.bv_val(1024, 16);
    z3::expr int32Expr = zcontext.bv_val(1024, 32);
    cout << "int2 : " << int2Expr  << endl <<
            "int4 : " << int4Expr  << endl <<
            "int8 : " << int8Expr  << endl <<
            "int8 : " << int8Expr  << endl <<
            "int16: " << int16Expr << endl <<
            "int32: " << int32Expr << endl;
} */

/* void Z3Helper::enum_sort_example() {
    std::cout << "enumeration sort example\n";
    const char * enum_names[] = { "aa", "bb", "cc" };
    z3::func_decl_vector enum_consts(zcontext);
    z3::func_decl_vector enum_testers(zcontext);
    z3::sort s = zcontext.enumeration_sort("enumT", 3, enum_names, enum_consts, enum_testers);
    // enum_consts[0] is a func_decl of arity 0.
    // we convert it to an expression using the operator()
    z3::expr a = enum_consts[0]();
    z3::expr b = enum_consts[1]();
    z3::expr c = enum_consts[2]();
    cout << "enum const [0]: " << enum_consts[0]() << "\n";
    cout << "enum testers [0]: " << enum_testers[0] << "\n";
    z3::expr y = zcontext.constant("y", enum_consts[0]().get_sort());
    cout << "y: " << y << "\n";
    z3::expr x = zcontext.constant("cc", s);
    z3::expr test = (y==b) || (y==c);
    std::cout << "1: " << test << std::endl;
    zsolver.add(test);
    cout << zsolver.check() << endl;
    z3::tactic qe(zcontext, "ctx-solver-simplify");
    z3::goal g(zcontext);
    g.add(test);
    z3::expr res(zcontext);
    z3::apply_result result_of_elimination = qe.apply(g);
    z3::goal result_goal = result_of_elimination[0];
    std::cout << "2: " << result_goal.as_expr() << std::endl;
} */

/* void unsat_core_example1() {
        std::cout << "unsat core example1\n";
        z3::context c;
        // We use answer literals to track assertions.
        // An answer literal is essentially a fresh Boolean marker
        // that is used to track an assertion.
        // For example, if we want to track assertion F, we 
        // create a fresh Boolean variable p and assert (p => F)
        // Then we provide p as an argument for the check method.
        z3::expr p1 = c.bool_const("p1");
        z3::expr p2 = c.bool_const("p2");
        z3::expr p3 = c.bool_const("p3");
        z3::expr x  = c.int_const("x");
        z3::expr y  = c.int_const("y");
        z3::solver s(c);
        s.add(implies(p1, x > 10));
        s.add(implies(p1, y > x));
        s.add(implies(p2, y < 5));
        s.add(implies(p3, y > 0));
        z3::expr assumptions[3] = { p1, p2, p3 };
        int result = s.check(3, assumptions);
        if(result == 0){
            std::cout << "Expression is unsat\n";
        } else if(result == 1){
            std::cout << "Expression is sat\n";
        } else {
            std::cout << "Expression is unknown\n";
        }
        z3::expr_vector core = s.unsat_core();
        std::cout << core << "\n";
        std::cout << "size: " << core.size() << "\n";
        for (unsigned i = 0; i < core.size(); i++) {
            std::cout << core[i] << "\n";
        }
        // Trying again without p2
        z3::expr assumptions2[2] = { p1, p3 };
        std::cout << s.check(2, assumptions2) << "\n";

		s.reset();
		z3::expr ex1 = p1 && p2;
		z3::expr ex2 = !p1 && p3;
		s.add(ex1);
		s.add(ex2);
		std::cout << s.check() << "\n";
		core = s.unsat_core();
        std::cout << core << "\n";
        std::cout << "size: " << core.size() << "\n";
        for (unsigned i = 0; i < core.size(); i++) {
            std::cout << core[i] << "\n";
        }
		
    } */
