/**
 * Implements functions in pred.hpp. The two sets of variables are encoded as follows:
 * The set referred to by u are encoded as idx*2 (even numbers), and v as idx*2+1 (odd numbers)
 */

#include "headers/pred.hpp"

#include <vector>
#include <stdexcept>

#include "headers/bdd.hpp"



/**
 * Impl StateSpace
 */
StateSpace::StateSpace(int st_bits) 
    : state_bits(st_bits), var_eq_bdd(false), cube_u(false), cube_v(false)
{
    std::vector<int> u_vars(state_bits), v_vars(state_bits);
    for(int i = 0; i < state_bits; i++)
    {
        var_eq_bdd |= BDD(i*2) ^ BDD(i*2+1);
        u_vars[i] = i * 2;
        v_vars[i] = i * 2 + 1;
    }
    var_eq_bdd = !var_eq_bdd; 
    cube_u = BDD(u_vars);
    cube_v = BDD(v_vars);
}

bool operator==(const StateSpace& sl, const StateSpace& sr) { return sl.state_bits == sr.state_bits;}
bool operator!=(const StateSpace& sl, const StateSpace& sr) { return sl.state_bits != sr.state_bits;}



/**
 * Impl Transaction
 */

// CTOR etc
Transition::Transition(const StateSpace& sp, const BDD& tuv, const BDD& tvu) 
    : space(sp), t_u_v(tuv), t_v_u(tvu) {} 

Transition::Transition(const StateSpace& sp, int var_idx, bool to_var) 
    : Transition(sp, BDD(var_idx * 2 + (to_var ? 1 : 0)), BDD(var_idx * 2 + (to_var ? 0 : 1))) {}

Transition::Transition(const StateSpace& sp, bool bconst) 
    : Transition(sp, BDD(bconst), BDD(bconst)) {}

Transition::Transition(const Transition& other) 
    : Transition(other.space, other.t_u_v, other.t_v_u) {}

Transition& Transition::operator=(const Transition& other)
{
    if(other.space != space)
        throw std::runtime_error("Cannot assign Transition over different StateSpaces");
    t_u_v = other.t_u_v;
    t_v_u = other.t_v_u;
    return *this;
}


// Equality
bool operator==(const Transition& trl, const Transition& trr)
{
    return trl.space == trr.space && trl.t_u_v == trr.t_u_v && trl.t_v_u == trr.t_v_u;
}
bool operator!=(const Transition& trl, const Transition& trr) { return !(trl == trr); }


// Operators
Transition  operator&&(const Transition& trl, const Transition& trr) 
{ 
    if(trl.space != trr.space) 
        throw std::runtime_error("Cannot operate on transition over different StateSpaces");
    return Transition(trl.space, trl.t_u_v && trr.t_u_v, trl.t_v_u && trr.t_v_u); 
}
Transition  operator||(const Transition& trl, const Transition& trr) 
{ 
    if(trl.space != trr.space) 
        throw std::runtime_error("Cannot operate on transition over different StateSpaces");
    return Transition(trl.space, trl.t_u_v || trr.t_u_v, trl.t_v_u || trr.t_v_u); 
}
Transition  operator^(const Transition& trl, const Transition& trr) 
{ 
    if(trl.space != trr.space) 
        throw std::runtime_error("Cannot operate on transition over different StateSpaces");
    return Transition(trl.space, trl.t_u_v ^ trr.t_u_v, trl.t_v_u ^ trr.t_v_u); 
}

Transition& Transition::operator&=(const Transition& other) { return *this = *this && other; }
Transition& Transition::operator|=(const Transition& other) { return *this = *this || other; }
Transition& Transition::operator^=(const Transition& other) { return *this = *this ^  other; }

Transition  Transition::operator! () { return Transition(space, !t_u_v, !t_v_u); }


// CTL operators
Predicate Transition::EX(const Predicate& pred) const
{
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    if(pred.is_p_u_repr)
        return Predicate(space, (t_v_u && pred.p_u).existential_abstraction(space.cube_u), false);
    else
        return Predicate(space, (t_u_v && pred.p_v).existential_abstraction(space.cube_v), true);
}
Predicate Transition::EF(const Predicate& pred) const
{
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, false);
    Predicate nxt(space, false);
    while((nxt = pred || EX(acc)) != acc) acc = nxt;
    return acc;
}
Predicate Transition::EG(const Predicate& pred) const
{
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, true);
    Predicate nxt(space, false);
    while((nxt = pred && EX(acc)) != acc) acc = nxt;
    return acc;
}
Predicate Transition::EU(const Predicate& predl, const Predicate& predr) const
{
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, true);
    Predicate nxt(space, false);
    while((nxt = predl || (predr && EX(acc))) != acc) acc = nxt;
    return acc;
}
Predicate Transition::ER(const Predicate& predl, const Predicate& predr) const
{
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, true);
    Predicate nxt(space, false);
    while((nxt = predr && (predl || EX(acc))) != acc) acc = nxt;
    return acc;
}
Predicate Transition::AX(const Predicate& pred) const
{
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    if(pred.is_p_u_repr)
        return Predicate(space, (!t_v_u || pred.p_u).universal_abstraction(space.cube_u), false);
    else
        return Predicate(space, (!t_u_v || pred.p_v).universal_abstraction(space.cube_v), true);
}
Predicate Transition::AF(const Predicate& pred) const
{
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, false);
    Predicate nxt(space, false);
    while((nxt = pred || AX(acc)) != acc) acc = nxt;
    return acc;
}
Predicate Transition::AG(const Predicate& pred) const
{
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, true);
    Predicate nxt(space, false);
    while((nxt = pred && AX(acc)) != acc) acc = nxt;
    return acc;
}
Predicate Transition::AU(const Predicate& predl, const Predicate& predr) const
{
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, true);
    Predicate nxt(space, false);
    while((nxt = predl || (predr && AX(acc))) != acc) acc = nxt;
    return acc;
}
Predicate Transition::AR(const Predicate& predl, const Predicate& predr) const
{
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, true);
    Predicate nxt(space, false);
    while((nxt = predr && (predl || AX(acc))) != acc) acc = nxt;
    return acc;
}



/**
 * Impl Predicate
 */ 

// CTOR etc
Predicate::Predicate(const StateSpace& sp, const BDD& repr, bool is_repr_u) 
    : space(sp), is_p_u_repr(is_repr_u), p_u(false), p_v(false)
{
   if(is_p_u_repr)     { p_u = repr;        p_v = BDD(false); }
   else                { p_u = BDD(false);  p_v = repr;       }
}

Predicate::Predicate(const StateSpace& sp, int var_idx) : Predicate(sp, BDD(var_idx*2), true) {}
Predicate::Predicate(const StateSpace& sp, bool bconst) : Predicate(sp, BDD(bconst),    true) {}
Predicate::Predicate(const Predicate& other) 
    : Predicate(other.space, other.is_p_u_repr ? other.p_u : other.p_v, other.is_p_u_repr)    {}

Predicate& Predicate::operator=(const Predicate& other)
{
    if(other.space != space)
        throw std::runtime_error("Cannot assign Predicates over seperate StateSpaces");
    is_p_u_repr = other.is_p_u_repr;
    if(is_p_u_repr) p_u = other.p_u;
    else            p_v = other.p_v;
    return *this;
}


// Operators
Predicate operator&&(const Predicate& predl, const Predicate& predr) 
{
    if(predl.space != predr.space) 
        throw std::runtime_error("Cannot operate on predicates on different spaces");
    const StateSpace& space = predl.space;
    if(!predl.is_p_u_repr && !predr.is_p_u_repr)
        return Predicate(space, predl.p_v && predr.p_v, false);
    BDD ddl = predl.is_p_u_repr ? predl.p_u 
                    : (predl.p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
    BDD ddr = predr.is_p_u_repr ? predr.p_u 
                    : (predr.p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
    return Predicate(space, ddl && ddr, true);
}

Predicate operator||(const Predicate& predl, const Predicate& predr) 
{
    if(predl.space != predr.space) 
        throw std::runtime_error("Cannot operate on predicates on different spaces");
    const StateSpace& space = predl.space;
    if(!predl.is_p_u_repr && !predr.is_p_u_repr)
        return Predicate(space, predl.p_v || predr.p_v, false);
    BDD ddl = predl.is_p_u_repr ? predl.p_u 
                    : (predl.p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
    BDD ddr = predr.is_p_u_repr ? predr.p_u 
                    : (predr.p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
    return Predicate(space, ddl || ddr, true);
}

Predicate operator^(const Predicate& predl, const Predicate& predr) 
{
    if(predl.space != predr.space) 
        throw std::runtime_error("Cannot operate on predicates on different spaces");
    const StateSpace& space = predl.space;
    if(!predl.is_p_u_repr && !predr.is_p_u_repr)
        return Predicate(space, predl.p_v ^ predr.p_v, false);
    BDD ddl = predl.is_p_u_repr ? predl.p_u 
                    : (predl.p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
    BDD ddr = predr.is_p_u_repr ? predr.p_u 
                    : (predr.p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
    return Predicate(space, ddl ^ ddr, true);
}

Predicate& Predicate::operator&=(const Predicate& other) { return *this = *this && other; } 
Predicate& Predicate::operator|=(const Predicate& other) { return *this = *this || other; } 
Predicate& Predicate::operator^=(const Predicate& other) { return *this = *this ^ other;  }

Predicate Predicate::operator!() { return Predicate(space, is_p_u_repr ? !p_u : !p_v, is_p_u_repr);}





// Equality
bool operator==(const Predicate& predl, const Predicate& predr)
{ 
    if(predl.space != predr.space) return false;
    const StateSpace& space = predl.space;
    BDD ddl = predl.is_p_u_repr ? predl.p_u : predl.p_v; 
    BDD ddr = predr.is_p_u_repr ? predr.p_u : predr.p_v;
    if (predl.is_p_u_repr == predr.is_p_u_repr) return ddl == ddr;
    return ((ddl ^ ddr) && space.var_eq_bdd) == BDD(false);

}
bool operator!=(const Predicate& predl, const Predicate& predr) { return !(predl == predr); }


// Getters
BDD Predicate::get_bdd() const
{
    if(is_p_u_repr) return p_u;
    return (p_v && space.var_eq_bdd).existential_abstraction(space.cube_v);
}


// Check if sat or valid
bool Predicate::is_true()  const { return (is_p_u_repr ? p_u : p_v).is_one();  }
bool Predicate::is_false() const { return (is_p_u_repr ? p_u : p_v).is_zero(); }



