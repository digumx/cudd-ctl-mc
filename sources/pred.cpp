/**
 * Implements functions in pred.hpp. The two sets of variables are encoded as follows:
 * The set referred to by u are encoded as idx*2 (even numbers), and v as idx*2+1 (odd numbers)
 */

#include "headers/pred.hpp"

#include "headers/bdd.hpp"



/**
 * Some util functions used later
 */

// converts a bdd in v variables to an equivalent bdd in u variables given an var_eq_bdd
inline BDD bdd_v2u(const BDD& var_eq, const BDD& v_bdd) { return }



/**
 * Impl StateSpace
 */
StateSpace::StateSpace(int state_bits) : state_bits(state_bits), var_eq_bdd(false)
{
    for(i = 0; i < state_bits; i++)
        var_eq_bdd |= BDD(i*2) ^ BDD(i*2+1);
    var_eq_bdd = !var_eq_bdd;
}



/**
 * Impl Transaction
 */

// CTOR etc
Transition::Transition(const BDD& tuv, const BDD& tvu) : t_u_v(tuv), t_v_v(tvu) {} 

Transition::Transition(int var_idx, bool to_var) 
    : Transition(BDD(var_idx * 2 + to_var ? 1 : 0), BDD(var_idx * 2 + to_var ? 0 : 1)) {}

Transition::Transition(bool bconst) : Transition(BDD(bconst), BDD(bconst)) {}

// Operators
Transition  operator&&(const Transition& trl, const Transition& trr) 
    { return Transition(trl.t_u_v && trr.t_u_v, trl.t_v_u && trr.t_v_u); }

Transition  operator||(const Transition& trl, const Transition& trr) 
    { return Transition(trl.t_u_v || trr.t_u_v, trl.t_v_u || trr.t_v_u); }

Transition  operator^(const Transition& trl, const Transition& trr) 
    { return Transition(trl.t_u_v ^  trr.t_u_v, trl.t_v_u ^  trr.t_v_u); }

Transition& Transition::operator&=(const Transition& other) { return *this = *this && other; }
Transition& Transition::operator|=(const Transition& other) { return *this = *this || other; }
Transition& Transition::operator^=(const Transition& other) { return *this = *this ^  other; }

Transition  Transition::operator! () { return Transition(!t_u_v, !t_v_u); }



/**
 * Impl Pred
 */ 

// CTOR etc
Predicate::Predicate(const StateSpace& sp, const BDD& repr, bool is_repr_u) 
    : space(sp), is_repr_correct(is_repr_u)
{
   if(is_repr_correct) { p_u = repr;        p_v = BDD(false); }
   else                { p_u = BDD(fasle)   p_v = repr;       }
}

Predicate::Predicate(const StateSpace& sp, int var_idx) : Predicate(sp, BDD(var_idx*2), true) {}
Predicate::Predicate(const StateSpace& sp, bool bconst) : Predicate(sp, BDD(bconst),    true) {}

// 

// Operators
Predicate operator&&(const Predicate& predl, const Predicate& predr) 
