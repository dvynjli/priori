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
    
    addInferenceRules();
}

void Z3Helper::addInferenceRules() {
    try {
        // cout << "init" << endl;
        zfp.register_relation(isLoad);
        zfp.register_relation(isStore);
        zfp.register_relation(isVarOf);
        zfp.register_relation(memOrderOf);
        zfp.register_relation(mhb);
        zfp.register_relation(rf);
        zfp.register_relation(nrf);
        zfp.register_relation(po);
        zfp.register_relation(moc);
        z3::expr inst1 = zcontext.bv_const("inst1", BV_SIZE);
        z3::expr inst2 = zcontext.bv_const("inst2", BV_SIZE);
        z3::expr inst3 = zcontext.bv_const("inst3", BV_SIZE);
        z3::expr inst4 = zcontext.bv_const("inst4", BV_SIZE);
        z3::expr inst5 = zcontext.bv_const("inst5", BV_SIZE);
        z3::expr var1  = zcontext.bv_const("var1", BV_SIZE);
        z3::expr var2  = zcontext.bv_const("var2", BV_SIZE);
        z3::expr ord1  = zcontext.int_const("ord1");
        // z3::expr app = mhb(inst1, inst2) && mhb(inst2, inst3);

        // (s1,s2) in PO && (s2,s3) in PO => (s1,s3) in PO
        z3::expr transitive_po = z3::forall(inst1, inst2, inst3, 
                z3::implies((po(inst1, inst2) && po(inst2, inst3)), 
                po(inst1, inst3) ));
        zfp.add_rule(transitive_po, zcontext.str_symbol("Transitive-PO"));
        
        // (s1,s2) in MHB && (s2,s3) in MHB => (s1,s3) in MHB
        z3::expr transitive_mhb = z3::forall(inst1, inst2, inst3, 
                z3::implies((mhb(inst1, inst2) && mhb(inst2, inst3)), 
                mhb(inst1, inst3) ));
        zfp.add_rule(transitive_mhb, zcontext.str_symbol("Transitive-MHB"));
        // cout << "Transitive MHB added" << endl;

        z3::expr nrf1 = z3::forall(inst1, inst2, 
                z3::implies( (isLoad(inst1) && isStore(inst2) && mhb(inst1, inst2)), 
                nrf(inst2, inst1)));
        zfp.add_rule(nrf1, zcontext.str_symbol("NRF1"));

        z3::expr nrf2 = z3::forall(inst1, inst2, inst3, var1,
                z3::implies( (isStore(inst1) && isStore(inst2) && isLoad(inst3) &&
                        isVarOf(inst1, var1) && isVarOf(inst2, var1) && isVarOf(inst3, var1) &&
                        mhb(inst1, inst2) && mhb(inst2, inst3)),
                nrf(inst1, inst3)));
        zfp.add_rule(nrf2, zcontext.str_symbol("NRF2"));

        // ( (l,op) \in PO && l \in AcqOp) => (l,op) \in moc
        z3::expr acqReordering = z3::forall(inst1, inst2, 
                z3::implies(po(inst1, inst2) && isLoad(inst1) && 
                    z3::exists(ord1, memOrderOf(inst1, ord1) && ord1>=ACQ),
                moc(inst1, inst2)));
        zfp.add_rule(acqReordering, zcontext.str_symbol("Acq-Reordering"));
        // cout << "Acq" << endl;
        
        // ( (op,s) \in PO && s \in RelOp) => (op,s) \in moc
        z3::expr relReordering = z3::forall(inst1, inst2,
                z3::implies(po(inst1, inst2) && isStore(inst2) && 
                    z3::exists(ord1, memOrderOf(inst2, ord1) && ord1>=REL),
                moc(inst1, inst2)));
        zfp.add_rule(relReordering, zcontext.str_symbol("Rel-Reordering"));
        // cout << "Rel" << endl;


        // ( s \in RelOp && l \in AcqOp && 
        //   (l,v1) \in IsLoad && (s,v1) \in IsStore && 
        //   (s,l) \in RF &&
        //   (op1,s) \in moc && (l,op2) \in moc)
        // => (op1,op2) \in MHB
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
                    moc(inst3, inst1) && moc(inst2, inst4), 
                mhb(inst3, inst4)));
        zfp.add_rule(relAcqSeq, zcontext.str_symbol("Rel-Acq-Seq1"));
        // cout << "Rel-acq" << endl;

        
        // (s2 \in RelOp && l \in AcqOp &&
        //   (s2,v1) \in isStore && (l,v1) \in isStore && (s1,v1) \in isStore
        //   (s1,l) \in RF && (s2,s1) \in PO &&
        //   (op1,s2) \in moc && (l,op2) \in moc)
        // => (op1,l) \in MHB
        xs.push_back(inst5);
        z3::expr relAcqSeq2 = z3::forall(xs, 
                z3::implies(isStore(inst1) && isLoad(inst2) && isStore(inst3) && 
                    z3::exists(ord1, memOrderOf(inst2, ord1) && ord1>=ACQ) && 
                    z3::exists(ord1, memOrderOf(inst3, ord1) && ord1>=REL) &&
                    isVarOf(inst1, var1) && isVarOf(inst2, var1) && isVarOf(inst3, var1) &&
                    rf(inst1, inst2) && po(inst3, inst1) &&
                    moc(inst4, inst3) && moc(inst2, inst5), 
                mhb(inst4, inst5)));
        zfp.add_rule(relAcqSeq2, zcontext.str_symbol("Rel-Acq-Seq2"));

        // add init (null) as store instruction of all var of all mem-order
        const z3::expr instExpr = getBitVec(nullptr);
        z3::expr app = isStore(instExpr);
        zfp.add_rule(app, zcontext.str_symbol("initIsStore"));
        app = z3::forall(var1, isVarOf(instExpr, var1));
        zfp.add_rule(app, zcontext.str_symbol("initOfAllVar"));
        app = z3::forall(ord1, memOrderOf(instExpr, ord1));
        zfp.add_rule(app, zcontext.str_symbol("initAllMemOrd"));

        // cout << "from init:\n" << zfp.to_string() << "\n";

        // z3::expr a = zcontext.bv_val(2, BV_SIZE);
        // z3::expr b = zcontext.bv_val(5, BV_SIZE);
        // Z3_fixedpoint_add_rule(zcontext, zfp, isVarOf(a,b), zcontext.str_symbol("test"));

        // z3::expr query1 = isVarOf(a,a);
        // z3::expr query2 = isVarOf(a,b);
        // cout << zfp.query(query1) << "\n";
        // cout << zfp.query(query2) << "\n";
        
    } catch (z3::exception e) {cout << "Exception: " << e << endl; exit(0);}
}

void Z3Helper::addPO (llvm::Instruction *from, llvm::Instruction *to) {
    const z3::expr fromExpr = getBitVec(from);
    const z3::expr toExpr = getBitVec(to);
    z3::expr app = po(fromExpr, toExpr);
    zfp.add_rule(app, zcontext.str_symbol(""));
    // rules = rules + "\n" +  app.to_string();
}

void Z3Helper::addMHB (llvm::Instruction *from, llvm::Instruction *to) {
    const z3::expr fromExpr = getBitVec(from);
    const z3::expr toExpr = getBitVec(to);
    // const z3::expr trueExpr = zcontext.bool_val(true);
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
    zfp.add_rule(app, zcontext.str_symbol(""));
    // rules = rules + "\n" +  app.to_string();
}

bool Z3Helper::checkInterference (unordered_map<llvm::Instruction*, llvm::Instruction*> interfs) {
    // cout << "All rules:\n" << zfp.to_string() << "\n";
    bool isFeasible = true;
    try {
        addInterference(interfs);
        z3::expr query = makeQueryOfInterference(interfs);
        isFeasible = !(zfp.query(query));
    } catch (z3::exception e) {
        cout << "Exception: " << e << endl;
        exit(0);
    }
    // cout << "Feasible: " << isFeasible << "\n";
    return isFeasible;
}

string Z3Helper::getRules() {
    return rules;
}

void Z3Helper::addRules(string rulesToAdd) {
    cout << "Before adding MHB:\n" << zfp.to_string() << endl;
    cout << "Trying to add:\n" << rulesToAdd << endl;
    try {
        zfp.from_string(rulesToAdd.c_str());
    } catch (z3::exception e) {cout << "Exception: "<< e << endl;}
    cout << "After adding MHB:\n" << zfp.to_string() << endl;
}

void Z3Helper::addMHBandPORules(vector< pair <string, pair<llvm::Instruction*, llvm::Instruction*>>> relations) {
    for (auto it=relations.begin(); it!=relations.end(); ++it) {
        auto instPair = it->second;
        if(it->first == "mhb") {
            addMHB(instPair.first, instPair.second);
        }
        else if (it->first == "po") {
            addPO(instPair.first, instPair.second);
        }
        else {
            cout << "something wrong: " << it->first << endl;
            exit(0);
        }
    }
}

void Z3Helper::addAllLoads(unordered_map<llvm::Function*, unordered_map<llvm::Instruction*, string>> allLoads) {
    for (auto funcItr=allLoads.begin(); funcItr!=allLoads.end(); ++funcItr) {
        auto loads = funcItr->second;
        for (auto loadsItr=loads.begin(); loadsItr!=loads.end(); ++loadsItr) {
            llvm::LoadInst *loadInst = llvm::dyn_cast<llvm::LoadInst>(loadsItr->first);
            addLoadInstr(loadInst);
        }
    }
}

void Z3Helper::addAllStores(unordered_map<llvm::Function*, unordered_map<string, unordered_set<llvm::Instruction*>>> allStores) {
    for (auto funcItr=allStores.begin(); funcItr!=allStores.end(); ++funcItr) {
        auto varTostores = funcItr->second;
        for (auto varItr=varTostores.begin(); varItr!=varTostores.end(); ++varItr) {
            unordered_set<llvm::Instruction*> stores = varItr->second;
            // while (!stores.empty()) {
            //     llvm::Instruction *inst = stores.;
            //     llvm::StoreInst *storeInst = llvm::dyn_cast<llvm::StoreInst>(inst);
            //     addStoreInstr(storeInst);
            // }
            for (auto storesItr=stores.begin(); storesItr!=stores.end(); ++storesItr) {
                auto inst = *storesItr;
                llvm::StoreInst *storeInst = llvm::dyn_cast<llvm::StoreInst>(inst);
                addStoreInstr(storeInst);
            }
        }
    }
}

void Z3Helper::addLoadInstr (llvm::LoadInst *inst) {
    const z3::expr instExpr = getBitVec(inst);
    z3::expr app = isLoad(instExpr);
    zfp.add_rule(app, zcontext.str_symbol(""));
    // rules = rules + "\n" +  app.to_string();
    
    llvm::Value* fromVar = inst->getOperand(0);
    addInstToVar(instExpr, fromVar);

    llvm::AtomicOrdering ord = inst->getOrdering();
    addInstToMemOrd(instExpr, ord);
}

void Z3Helper::addStoreInstr (llvm::StoreInst *inst) {
    const z3::expr instExpr = getBitVec(inst);
    z3::expr app = isStore(instExpr);
    zfp.add_rule(app, zcontext.str_symbol(""));
    // rules = rules + "\n" +  app.to_string();
    
    llvm::Value* fromVar = inst->getPointerOperand();
    addInstToVar(instExpr, fromVar);

    llvm::AtomicOrdering ord = inst->getOrdering();
    addInstToMemOrd(instExpr, ord);
}

void Z3Helper::addInstToVar(z3::expr inst, llvm::Value *var) {
    const z3::expr varExpr = getBitVec(var);
    z3::expr app = isVarOf(inst, varExpr);
    zfp.add_rule(app, zcontext.str_symbol(""));
    // rules = rules + "\n" +  app.to_string();
}

void Z3Helper::addInstToMemOrd(z3::expr inst, llvm::AtomicOrdering ord) {
    const z3::expr ordExpr = getMemOrd(ord);
    z3::expr app = memOrderOf(inst, ordExpr);
    zfp.add_rule(app, zcontext.str_symbol(""));
    // rules = rules + "\n" +  app.to_string();
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
            cout << "WARNING: Unknown mem order" << endl;
            break;
    }
    return zcontext.int_val(ordInt);
}

void Z3Helper::addInterference (unordered_map<llvm::Instruction*, llvm::Instruction*> interfs) {
    // zfp.push();
    // rules = zfp.to_string();
    int interfCount = 0;
    for (auto it=interfs.begin(); it!=interfs.end(); ++it) {
        // cout << "adding interf" << interfCount << endl;
        z3::expr rfExpr = rf(getBitVec(it->second), getBitVec(it->first));
        zfp.add_rule(rfExpr, zcontext.str_symbol(("Interf"+std::to_string(interfCount)).c_str()));
        interfCount++;
    }
}

z3::expr Z3Helper::makeQueryOfInterference (unordered_map<llvm::Instruction*, llvm::Instruction*> interfs) {
    auto it=interfs.begin();
    z3::expr query = nrf(getBitVec(it->second), getBitVec(it->first));
    it++;
    for (; it!=interfs.end(); ++it) {
        z3::expr tmp = nrf(getBitVec(it->second), getBitVec(it->first));
        query = query || tmp;
    }
    return query;
}

/* void Z3Helper::removeInterference () {
    cout << "pop" <<endl;
    // zfp.pop();
    zfp.~fixedpoint();
    zfp = z3::fixedpoint(zcontext);
    cout << "removal done" << endl;
    
    // int pos=0;
    // for (int i=0; i<8; i++) {
    //     pos = rules.find('\n', pos);
    //     cout << pos << " ";
    //     pos++;
    // }
    // cout << "\n";
    // // rules = rules.substr(pos);

    cout << "rules from str" << endl;

    // cout << rules << endl;
    try {
        addInferenceRules();
        // zfp.from_string(rules.c_str());
    } catch (z3::exception e) {cout << "Exception: " << e << endl;}
    cout << "Added rules to new zfp" << endl;
} */

void Z3Helper::testQuery() {
    z3::expr nullInst = getBitVec(nullptr);
    
    z3::expr query = isVarOf(nullInst, zcontext.bv_val(6, BV_SIZE));
    cout << "Query: " << query << endl;
    
    bool isFeasible = zfp.query(query);
    enum z3::check_result res = zfp.query(query);
    cout << "Query is: " << isFeasible << "res: " << res << "\n" << endl;

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
