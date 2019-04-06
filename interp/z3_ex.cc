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
    zfp.register_relation(edge);
    z3::func_decl path = z3::function("path", s, s, B);
    zfp.register_relation(path);

    z3::expr a = ctx.bv_const("a", 3);
    z3::expr b = ctx.bv_const("b", 3);
    z3::expr c = ctx.bv_const("c", 3);
    z3::expr t = ctx.bool_val(true);
    z3::expr f = ctx.bool_val(false);

	try {
        z3::expr rule1 = z3::implies(edge(a,b), path(a,b));
        // zsolver.add(rule1);
        // zfp.add_rule(rule1, ctx.str_symbol("edgeIsPath"));
        Z3_fixedpoint_add_rule(ctx, zfp, z3::forall(a,b, rule1), ctx.str_symbol("edgeIsPath"));
        z3::expr rule2 = z3::implies( (path(a,b) && path(b,c)), path(a,c) );
        // zfp.add_rule(rule2, ctx.str_symbol("transitivity"));
        Z3_fixedpoint_add_rule(ctx, zfp, z3::forall(a,b,c, rule2), ctx.str_symbol("transitivity"));

        z3::expr rule3 =z3::implies(edge(a,b), edge(b,a)==f);
        // Z3_fixedpoint_add_rule(ctx, zfp, z3::forall(a,b, rule3), ctx.str_symbol("anti-reflexive"));

        z3::expr n1 = ctx.bv_val(1,3);
        z3::expr n2 = ctx.bv_val(2,3);
        z3::expr n3 = ctx.bv_val(3,3);
        z3::expr n4 = ctx.bv_val(4,3);

        z3::expr e1 = edge(n1,n2);
        z3::expr e2 = edge(n1,n3);
        z3::expr e3 = edge(n2,n4);
        z3::expr e4 = edge(n4,n1);

        // zsolver.add(edge(n1,n2));
        // zsolver.add(edge(n1,n3));
        // zsolver.add(edge(n2,n4));

        // zfp.add_rule(e1, ctx.str_symbol("1_to_2"));
        // zfp.add_rule(e2, ctx.str_symbol("1_to_3"));
        // zfp.add_rule(e3, ctx.str_symbol("2_to_4"));

        Z3_fixedpoint_add_rule(ctx, zfp, e1, ctx.str_symbol("1_to_2"));
        Z3_fixedpoint_add_rule(ctx, zfp, e2, ctx.str_symbol("1_to_3"));
        Z3_fixedpoint_add_rule(ctx, zfp, e3, ctx.str_symbol("2_to_4"));
        Z3_fixedpoint_add_rule(ctx, zfp, e4, ctx.str_symbol("4_to_2"));

        
        // z3::func_decl q1= z3::function("q1", 0, NULL, ctx.bool_sort());
        // zfp.register_relation(q1);
        // z3::expr q1 = ctx.bool_const("q1");
        // cout << q1.to_string() << "\n";
        // z3::expr query1 = z3::implies(edge(n1,n2), q1);
        // zfp.add_rule(query1, ctx.str_symbol("query1"));
        // zsolver.add(query1);
        // Z3_fixedpoint_add_rule(ctx, zfp, z3::exists(q1, query1), NULL);
        // cout << "\nZ3 Solver: \n" << zsolver.to_smt2() << "\n";
        cout << "\nFixed point: \n" << zfp.to_string() << "\n";
        
        // z3::expr tmp = q1;
        // z3::expr tmp[] = {q1(),};
        z3::expr tmp = path(n3,n2);
        enum z3::check_result res = zfp.query(tmp);
        // enum z3::check_result res = zsolver.check(1, tmp);
        cout << res << "\n";
        
        // Z3_lbool result = Z3_fixedpoint_query(ctx, zfp, q1);
        // if (result == Z3_L_UNDEF)
        //     cout << "undefined\n";
        // else if (result == Z3_L_FALSE)
        //     cout << "unsat\n";
        // else if (result == Z3_L_TRUE)
        //     cout << "sat\n";

    } catch (z3::exception e) { cout << "Exception: " << e << "\n";}


	return 0;
}

/*  encoding
    const char *smtlib_str = "(set-option :fixedpoint.engine datalog)    \
    (define-sort s () (_ BitVec 3))             \
    (declare-rel edge (s s))                    \
    (declare-rel path (s s))                    \
    (declare-var a s)                           \
    (declare-var b s)                           \
    (declare-var c s)                           \
                                                \
    (rule (=> (edge a b) (path a b)))           \
    (rule (=> (and (path a b) (path b c)) (path a c)))  \
                                                \
    (rule (edge #b001 #b010))                   \
    (rule (edge #b001 #b011))                   \
    (rule (edge #b010 #b100))                   \
                                                \
    (declare-rel q1 ())                         \
    (declare-rel q2 ())                         \
    (declare-rel q3 (s))                        \
    (rule (=> (path #b001 #b100) q1))           \
    (rule (=> (path #b011 #b100) q2))           \
    (rule (=> (path #b001 b) (q3 b)))           \
                                                \
    (query q1)                                  \
    (query q2)                                  \
    (query q3 :print-answer true)";

    // cout << ctx.parse_string(smtlib_str);
    Z3_ast_vector res = Z3_fixedpoint_from_string(ctx, zfp, smtlib_str);
    Z3_ast query1 = Z3_ast_vector_get(ctx, res, 1);
    cout << Z3_ast_vector_to_string(ctx, res) << "\n";
    cout << "AST: " << Z3_ast_to_string(ctx, query1) << "\n";

    cout << "\nFixed point: \n" << zfp.to_string() << "\n";
    Z3_lbool result = Z3_fixedpoint_query(ctx, zfp, query1);
    if (result == Z3_L_UNDEF)
        cout << "undefined\n";
    else if (result == Z3_L_FALSE)
        cout << "unsat\n";
    else if (result == Z3_L_TRUE)
        cout << "sat\n";
*/
/* correct smtlib
    Fixed point: 
    (declare-fun path ((_ BitVec 3) (_ BitVec 3)) Bool)
    (declare-fun edge ((_ BitVec 3) (_ BitVec 3)) Bool)
    (declare-fun q1 () Bool)
    (declare-fun q2 () Bool)
    (declare-fun q3 ((_ BitVec 3)) Bool)
    (declare-rel path ((_ BitVec 3) (_ BitVec 3)))
    (declare-rel q1 ())
    (declare-rel edge ((_ BitVec 3) (_ BitVec 3)))
    (declare-rel q2 ())
    (declare-rel q3 ((_ BitVec 3)))
    (declare-var A (_ BitVec 3))
    (declare-var B (_ BitVec 3))
    (declare-var C (_ BitVec 3))
    (rule (=> (edge B A) (path B A)))
    (rule (=> (and (path B C) (path C A)) (path B A)))
    (rule (edge #b001 #b010))
    (rule (edge #b001 #b011))
    (rule (edge #b010 #b100))
    (rule (=> (path #b001 #b100) q1))
    (rule (=> (path #b011 #b100) q2))
    (rule (=> (path #b001 A) (q3 A)))

    query q2
    unsat
*/
