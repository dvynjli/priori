void unsat_core_example1() {
    std::cout << "unsat core example1\n";
    context c;
    // We use answer literals to track assertions.
    // An answer literal is essentially a fresh Boolean marker
    // that is used to track an assertion.
    // For example, if we want to track assertion F, we 
    // create a fresh Boolean variable p and assert (p => F)
    // Then we provide p as an argument for the check method.
    expr p1 = c.bool_const("p1");
    expr p2 = c.bool_const("p2");
    expr p3 = c.bool_const("p3");
    expr x  = c.int_const("x");
    expr y  = c.int_const("y");
    solver s(c);
    s.add(implies(p1, x > 10));
    s.add(implies(p1, y > x));
    s.add(implies(p2, y < 5));
    s.add(implies(p3, y > 0));
    expr assumptions[3] = { p1, p2, p3 };
    std::cout << s.check(3, assumptions) << "\n";
    expr_vector core = s.unsat_core();
    std::cout << core << "\n";
    std::cout << "size: " << core.size() << "\n";
    for (unsigned i = 0; i < core.size(); i++) {
        std::cout << core[i] << "\n";
    }
    // Trying again without p2
    expr assumptions2[2] = { p1, p3 };
    std::cout << s.check(2, assumptions2) << "\n";
}

void enum_sort_example() {
    std::cout << "enumeration sort example\n";
    context ctx;
    const char * enum_names[] = { "a", "b", "c" };
    func_decl_vector enum_consts(ctx);
    func_decl_vector enum_testers(ctx);
    sort s = ctx.enumeration_sort("enumT", 3, enum_names, enum_consts, enum_testers);
    // enum_consts[0] is a func_decl of arity 0.
    // we convert it to an expression using the operator()
    expr a = enum_consts[0]();
    expr b = enum_consts[1]();
    expr x = ctx.constant("x", s);
    expr test = (x==a) && (x==b);
    std::cout << "1: " << test << std::endl;
    tactic qe(ctx, "ctx-solver-simplify");
    goal g(ctx);
    g.add(test);
    expr res(ctx);
    apply_result result_of_elimination = qe.apply(g);
    goal result_goal = result_of_elimination[0];
    std::cout << "2: " << result_goal.as_expr() << std::endl;
}


/*
useful links
detailed tutorial (not c++): http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.225.8231&rep=rep1&type=pdf
an example: https://github.com/Z3Prover/z3/blob/master/examples/c%2B%2B/example.cpp
z3 fixpoints (not c++): https://rise4fun.com/z3/tutorialcontent/fixedpoints
short notes: http://www.cs.utah.edu/~vinu/research/formal/tools/notes/z3-notes.html
*/

