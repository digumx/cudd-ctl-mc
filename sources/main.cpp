/**
 * The main file, compliles to an executable
 */

#include <iostream>

#include "cudd.h"

#include "headers/bdd.hpp"
#include "headers/pred.hpp"


//int main(int argc, char** argv)
//{
//    /*BDD dd1(false);
//    BDD dd2(0);
//    dd1 |= dd2;
//    dd1.save_dot("./out/dbg.dot");*/
//
//    DdManager* man = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
//    DdNode* dd1 = Cudd_ReadLogicZero(man);
//    Cudd_Ref(dd1);
//    DdNode* dd2 = Cudd_bddIthVar(man, 0);
//    Cudd_Ref(dd2);
//    DdNode* dd3 = Cudd_bddOr(man, dd1, dd2);
//    Cudd_Ref(dd3);
//}


/*
 * For now, the main function simply computes EX, EXEX and AX for the transition function for a mod 2
 * counter, and checks EXp = !p, EXEXp = p
 */
int main(int argc, char** argv)
{
    // We define our state space to be over one bit
    StateSpace sp(1);

    // We firstly define p, q as predicates over sp
    Predicate p = !Predicate(sp, 0);                                            // p = v0<=>0 = !v0
    Predicate q = !p;                                                           // q = !p

    // Then we define the transition relation over sp
    Transition trans = Transition(sp, 0, false) ^ Transition(sp, 0, true);      // T(u, v) := u0^v0 

    // Now, we get the predicates for EXp, AXp and EXEXp.
    Predicate EXp = p.EX(trans);
    Predicate AXp = p.AX(trans);
    Predicate EXEXp = (p.EX(trans)).EX(trans);
    
    // Print out all BDDs generated into dot files
    p.get_bdd().save_dot("./out/p.dot");
    q.get_bdd().save_dot("./out/q.dot");
    EXp.get_bdd().save_dot("./out/EXp.dot");
    AXp.get_bdd().save_dot("./out/AXp.dot");
    EXEXp.get_bdd().save_dot("./out/EXEXp.dot");

    // Now we check if EXp <=> q and EXEXp <=> p. As EXp(v0) vs some other vi is just the same
    // function with different names for bound variables, we can just check EXp(v0) <=> q(v0) and so
    // on
    std::cout << "EXp <=> q "       << (EXp == q   ? "holds" : "does not hold") << std::endl;
    std::cout << "EX(EXp) <=> p "   << (EXEXp == p ? "holds" : "does not hold") << std::endl;
}
