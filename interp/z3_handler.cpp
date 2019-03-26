#include "z3_handler.h"

void Z3Helper::initZ3(vector<string> globalVars) {
    // TODO: declare enum for mem order and variable (if possible).
    // and change function varOf and memOrder accordingly
    // might need to define <=  for acq and rel mem orders
    const char * memOrderNames[] = { "rlx", "acq", "rel", "acq-rel", "seq_cst" };
    z3::func_decl_vector memOrderEnumConsts(zcontext);
    z3::func_decl_vector memOrderEnumTesters(zcontext);
    z3::sort memOrderSort = zcontext.enumeration_sort("memOrder", 5, memOrderNames, memOrderEnumConsts, memOrderEnumTesters);

    // TODO: should vars be considered constd instead of enum? will it improve performance?
    int noOfVars = globalVars.size();
    const char * varNames[noOfVars];
    for (int i=0; i<noOfVars; i++) {
        varNames[i] = globalVars[i].c_str();
    }
    z3::func_decl_vector varsEnumConsts(zcontext);
    z3::func_decl_vector varsEnumTesters(zcontext);
    z3::sort vars = zcontext.enumeration_sort("vars", noOfVars, varNames, varsEnumConsts, varsEnumTesters);

    // functions
    // isLoad: instr -> bool
    z3::func_decl isLoad = z3::function("isLoad", zcontext.int_sort(), zcontext.bool_sort());
    // isStore: instr -> bool
    z3::func_decl isStore = z3::function("isStore", zcontext.int_sort(), zcontext.bool_sort());
    // varOf: instr -> var
    z3::func_decl varOf = z3::function("varOf", zcontext.int_sort(), vars);
    // memOrderOf: instr -> memOrder
    z3::func_decl memOrderOf = z3::function("memOrder", zcontext.int_sort(), memOrderSort);

    // relations
    // MHB: does a MHB b? (instr, instr) -> bool
    z3::func_decl mhb = z3::function("MHB", zcontext.int_sort(), zcontext.int_sort(), zcontext.bool_sort());
    // RF: des a RF b? (instr, instr) -> bool
    z3::func_decl rf = z3::function("RF", zcontext.int_sort(), zcontext.int_sort(), zcontext.bool_sort());

    enum_sort_example();

}

void Z3Helper::enum_sort_example() {
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
    z3::expr x = zcontext.constant("x", s);
    x=c;
    z3::expr test = (x==a) || (x==b);
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
}

void Z3Helper::addMHB(llvm::Instruction *from, llvm::Instruction *to) {

}


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
