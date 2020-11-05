/**
 * Implements functions in pred.hpp. The two sets of variables are encoded as follows:
 * The set referred to by u are encoded as idx*2 (even numbers), and v as idx*2+1 (odd numbers)
 */

#include "headers/pred.hpp"

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>

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
 * Impl State
 */
State::State(const StateSpace& sp, const std::vector<bool>& assgn) 
    : space(sp), assign(assgn), bdd_u(true), bdd_v(true)
{
    if(sp.state_bits != (int) assign.size())
        throw std::runtime_error("Size of state assignment and state space do not match");
    for(size_t i = 0; i < assign.size(); ++i)
    {    
        bdd_u &= assign[i] ? BDD((int) i*2) : !BDD((int) i*2);
        bdd_v &= assign[i] ? BDD((int) i*2 + 1) : !BDD((int) i*2 + 1);
    }
}

State::State(const Predicate& pred) 
    : space(pred.space), assign(pred.space.state_bits), bdd_u(true), bdd_v(true)
{
    if(pred.is_false()) throw std::runtime_error("Cannot assign state from empty predicate");
    std::vector<bool> coded_assign = pred.get_bdd().get_assign();       // This in u vars, ignore v
    for(size_t i = 0; i < assign.size(); ++i)
    {
        assign[i] = coded_assign[2*i];
        bdd_u &= assign[i] ? BDD((int) i*2) : !BDD((int) i*2);
        bdd_v &= assign[i] ? BDD((int) i*2 + 1) : !BDD((int) i*2 + 1);
    }
}

State& State::operator = (const State& other)
{
    if(space != other.space) 
        throw std::runtime_error("Attempting to assign states over different spaces");
    assign = other.assign;
    bdd_u = other.bdd_u;
    bdd_v = other.bdd_v;
    return *this;
}

bool State::operator == (const State& other)
{
    if(space != other.space || assign.size() != other.assign.size()) return false;
    for(size_t i = 0; i < assign.size(); i++) if(assign[i] != other.assign[i]) return false;
    return true;
}

std::string State::to_string(size_t n_space) const
{
    std::string str = "";
    for(bool v : assign) str += (v ? "1" : "0") + std::string(n_space, ' ');
    return str;
}

 

/**
 * Impl path
 */
void Path::print() const
{
    // header
    std::cout << (is_finite ? "Finite" : "Infinite") << " path:" << std::endl;
    for(int i = 0; i < states[0].space.state_bits; i++)
    {
        std::string num = std::to_string(i);
        std::cout << "v" << num << std::string(3 - num.size(), ' ');
    }
    std::cout << std::endl;
    // Print before lasso (or whole path if finite)
    size_t brk = is_finite ? states.size() : lasso_point;
    for(size_t i = 0; i < brk; ++i)
        std::cout << states[i].to_string(3) << std::endl;
    // Print the lasso
    if(!is_finite) std::cout << "Begin Loop" << std::endl;
    for(size_t i = brk; i < states.size(); ++i)
        std::cout << states[i].to_string(3) << std::endl;
}




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

Transition  Transition::operator! () const { return Transition(space, !t_u_v, !t_v_u); }


// Get next
Predicate Transition::next(const State& st) const
{
    if(space != st.space) throw std::runtime_error("Spaces of state and transition do not match");
    return Predicate(space, (t_u_v && st.bdd_u).existential_abstraction(space.cube_u), false);
}

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
    Predicate acc(space, false);
    Predicate nxt(space, true);
    while((nxt = predr || (predl && EX(acc))) != acc) acc = nxt;
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
    Predicate acc(space, false);
    Predicate nxt(space, true);
    while((nxt = predr || (predl && AX(acc))) != acc) acc = nxt;
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



// Add fairness constraints
void Transition::add_fairness(const Predicate& pred) { fairness.push_back(pred); }

// Fair versions of the operators above
Predicate Transition::EX_fair(const Predicate& pred) const
{
    if(fairness.empty())
        throw std::runtime_error("Transition has no fairness conditions for fair quantifier");
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return EX(EG_fair(Predicate(space, true)) && pred); 
}
Predicate Transition::EF_fair(const Predicate& pred) const
{   
    if(fairness.empty())
        throw std::runtime_error("transition has no fairness conditions for fair quantifier");
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return EF(EG_fair(Predicate(space, true)) && pred); 
}
Predicate Transition::EG_fair(const Predicate& pred) const
{
    if(fairness.empty())
        throw std::runtime_error("Transition has no fairness conditions for fair quantifier");
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    Predicate acc(space, false);
    Predicate nxt(space, true);
    while(nxt != acc) 
    {
        acc = nxt;
        nxt = pred;
        for(std::vector<Predicate>::const_iterator i = fairness.begin(); i != fairness.end(); ++i)
            nxt &= EX(EU(pred, *i && acc));
    }
    return acc;
}
Predicate Transition::EU_fair(const Predicate& predl, const Predicate& predr) const
{   
    if(fairness.empty())
        throw std::runtime_error("Transition has no fairness conditions for fair quantifier");
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return EU(predl, EG_fair(Predicate(space, true)) && predr); 
}
Predicate Transition::ER_fair(const Predicate& predl, const Predicate& predr) const
{   
    if(fairness.empty())
        throw std::runtime_error("Transition has no fairness conditions for fair quantifier");
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return ER(EG_fair(Predicate(space, true)) && predl, predr); 
}
Predicate Transition::AX_fair(const Predicate& pred) const
{   
    if(fairness.empty())
        throw std::runtime_error("transition has no fairness conditions for fair quantifier");
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return !EX_fair(!pred); 
}
Predicate Transition::AF_fair(const Predicate& pred) const
{   
    if(fairness.empty())
        throw std::runtime_error("transition has no fairness conditions for fair quantifier");
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return !EG_fair(!pred); 
}
Predicate Transition::AG_fair(const Predicate& pred) const
{
    if(fairness.empty())
        throw std::runtime_error("transition has no fairness conditions for fair quantifier");
    if(space != pred.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return !EF_fair(!pred);
}
Predicate Transition::AU_fair(const Predicate& predl, const Predicate& predr) const
{   
    if(fairness.empty())
        throw std::runtime_error("Transition has no fairness conditions for fair quantifier");
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return !ER_fair(!predl, !predr); 
}
Predicate Transition::AR_fair(const Predicate& predl, const Predicate& predr) const
{   
    if(fairness.empty())
        throw std::runtime_error("Transition has no fairness conditions for fair quantifier");
    if(space != predl.space || space != predr.space) 
        throw std::runtime_error("Transition and predicate state spaces do not match");
    return !EU(!predl, !predr); 
}


// Witness or CEX generation
Path Transition::gen_witness_EF(const Predicate& init, const Predicate& EFf,
                                const Predicate& f) const
{
    return gen_witness_EU(init, EFf, Predicate(init.space, true), f);
}
Path Transition::gen_witness_EG(const Predicate& init, const Predicate& EGf, 
                                const Predicate& f) const
{
    Path ret; ret.is_finite = false;
    State st(init);
    std::vector<State>::iterator loc;
    // Keep generating a long path with states in EGf until it lassos
    while((loc = std::find(ret.states.begin(), ret.states.end(), st)) == ret.states.end())
    {
        ret.states.push_back(st);
        st = State(next(st) && EGf);        // This will always be nonempty by definition of EGf
    }
    ret.lasso_point = loc - ret.states.begin();
    return ret;
}
Path Transition::gen_witness_EU(const Predicate& init, const Predicate& EfUg, const Predicate& f, 
                                const Predicate& g) const
{
    Predicate nxt = init;
    Predicate end = nxt && g;
    Path ret; ret.is_finite = true;
    Predicate allowed(init.space, true);       // Tracks which states have not been visited yet
    // This loop produces paths with non-repeating vertices from EfUg, as g is in EfUg, such a path
    // will hit g eventually
    while((end = (nxt && g)).is_false())
    {
        State st(nxt);
        nxt = next(st) && EfUg && allowed;
        allowed &= !Predicate(st);
        ret.states.push_back(st);
    }
    ret.states.push_back(State(end));
    return ret;
}
Path Transition::gen_witness_ER(const Predicate& init, const Predicate& EfRg, const Predicate& f, 
                                const Predicate& g) const
{
    return gen_witness_EU(init, EfRg, f, f && g);
}
Path Transition::gen_cex_AF(const Predicate& init, const Predicate& AFf, 
                                const Predicate& f) const
{
    return gen_witness_EG(init, !AFf /*=EG!f*/, !f);
}
Path Transition::gen_cex_AG(const Predicate& init, const Predicate& AGf, 
                                const Predicate& f) const
{
    return gen_witness_EF(init, !AGf /*=EF!f*/, !f);
}
Path Transition::gen_cex_AU(const Predicate& init, const Predicate& AfUg, const Predicate& f, 
                                const Predicate& g) const
{
    return gen_witness_ER(init, !AfUg /*=E!fR!g*/, !f, !g);
}
Path Transition::gen_cex_AR(const Predicate& init, const Predicate& AfRg, const Predicate& f, 
                                const Predicate& g) const
{
    return gen_witness_EU(init, !AfRg /*=E!fU!g*/, !f, !g);
}





/**
 * Impl Predicate
 */ 

// CTOR etc
Predicate::Predicate(const StateSpace& sp, const BDD& repr, bool is_repr_u) 
    : space(sp), p_u(false), p_v(false), is_p_u_repr(is_repr_u)
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

Predicate::Predicate(const State& st) : Predicate(st.space, st.bdd_u, true) {}


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

Predicate Predicate::operator!() const { return Predicate(space, is_p_u_repr ? !p_u : !p_v, is_p_u_repr);}


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



