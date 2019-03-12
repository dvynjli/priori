#include "z3_handler.h"


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
