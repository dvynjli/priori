#include "z3++.h"

using namespace std;

int main() {
	z3::context ctx;
	z3::solver zsolver(ctx);
	z3::fixedpoint zfp(ctx);

	z3::params params(ctx);
	params.set("engine", ctx.str_symbol("datalog"));

	z3::sort s = ctx.bv_sort(3);
    z3::sort B = ctx.bool_sort();
    z3::func_decl edge = z3::function("edge", s, s, B);
    z3::func_decl path = z3::function("path", s, s, B);
    z3::expr a = ctx.bv_const("a", 3);
    z3::expr b = ctx.bv_const("b", 3);
    z3::expr c = ctx.bv_const("c", 3);
    z3::expr t = ctx.bool_val(true);
    z3::expr f = ctx.bool_val(false);

	try {
        // z3::expr rule1 = z3::implies((edge(a,b)==t), (path(a,b)==t));
        // Z3_fixedpoint_add_rule(ctx, zfp, rule1, NULL);
        // z3::expr rule2 = z3::implies( (path(a,b)==t && path(b,c)==t), path(a,c)==t );
        // Z3_fixedpoint_add_rule(ctx, zfp, rule2, NULL);

        z3::expr n1 = ctx.bv_val(1,3);
        z3::expr n2 = ctx.bv_val(2,3);
        z3::expr n3 = ctx.bv_val(3,3);
        z3::expr n4 = ctx.bv_val(4,3);

        Z3_fixedpoint_add_rule(ctx, zfp, edge(n1,n2)==t, NULL);
        Z3_fixedpoint_add_rule(ctx, zfp, edge(n1,n3)==t, NULL);
        Z3_fixedpoint_add_rule(ctx, zfp, edge(n2,n4)==t, NULL);

        z3::expr q1 = ctx.bool_const("q1");
        Z3_fixedpoint_add_rule(ctx, zfp, z3::implies(path(n1,n4)==t, q1), NULL);
        // Z3_fixedpoint_add_rule(ctx, zfp, (path(n1,n4)==q1), NULL);

        cout << "\nFixed point: \n" << zfp.to_string() << "\n";
        Z3_lbool result = Z3_fixedpoint_query(ctx, zfp, q1);
        if (result == Z3_L_UNDEF)
            cout << "undefined\n";
        else if (result == Z3_L_FALSE)
            cout << "unsat\n";
        else if (result == Z3_L_TRUE)
            cout << "sat\n";

    } catch (z3::exception e) { cout << "Exception: " << e << "\n";}


	return 0;
}