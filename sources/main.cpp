/**
 * The main file, compliles to an executable
 */

#include <iostream>

#include "cudd.h"
#include "headers/bdd.h"

DdManager* global_manager;

/** 
 * While BDDs do represent boolean functions, they are not a representation that is very directly
 * invariant under renaming of bound variables. This combined with the fact that renaming variables
 * in a BDD is potentially complicated (and not directly available via the CUDD interface), we avoid
 * the issue by representing boolean functions as objects of type Var->BDD. The following define p
 * and q as such objects.
 */
BDD p(int var_idx) { return !BDD(global_manager, var_idx); }           // p = v <=> 0 = !v
BDD q(int var_idx) { return !p(var_idx); }                                  // p = !q

/**
 * Similarly, we define the transition relation for the mod 2 counter as an object of type
 * Var->Var->BDD.
 */
BDD trans(int v1_idx, int v2_idx) 
{ 
    return BDD(global_manager, v1_idx) ^ BDD(global_manager, v2_idx);  // x <=> !y = x^y
}

/**
 * For now, the main function simply computes EX and AX for the transition function for a mod 2
 * counter
 */
int main(int argc, char** argv)
{
    // Init manager
    global_manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

    // We firstly define p, q as boolean function on var 0:
    BDD p_v0 = p(0);
    BDD q_v0 = q(0);

    // Now, we define EXp as a predicate on v0. Note that EXp(v0) Ev1:trans(v0,v1) and p(v1).
    BDD EXp_v0 = (trans(0, 1) && p(1)).existential_abstraction(1);
    // Similary for AXp(v0) and EX(EXp)(v0)
    BDD AXp_v0 = (trans(0, 1) && p(1)).universal_abstraction(1);
    BDD EXEXp_v0 = (trans(0, 1) && 
                        (trans(1, 2) && p(2)).existential_abstraction(2)
                    ).existential_abstraction(1);

    // Print out all BDDs generated into dot files
    p_v0.save_dot("./out/p_v0.dot");
    q_v0.save_dot("./out/q_v0.dot");
    EXp_v0.save_dot("./out/EXp_v0.dot");
    AXp_v0.save_dot("./out/AXp_v0.dot");
    EXEXp_v0.save_dot("./out/EXEXp_v0.dot");

    // Now we check if EXp <=> q and EXEXp <=> p. As EXp(v0) vs some other vi is just the same
    // function with different names for bound variables, we can just check EXp(v0) <=> q(v0) and so
    // on
    std::cout << "EXp <=> q "       << (EXp_v0 == q_v0   ? "holds" : "does not hold") << std::endl;
    std::cout << "EX(EXp) <=> p "   << (EXEXp_v0 == p_v0 ? "holds" : "does not hold") << std::endl;
}
