/**
 * This file defines classes that represent predicates on states represented as boolean functions on
 * bitvectors.
 *
 * To allow predicate transformers to existentially quantify over states and avoid renaming
 * variables, we maintain two representations for the predicate as BDDs over two sets of variables.
 * Existential quantification then converts a BDD over one set of variables to a BDD over another
 * set of variables.
 *
 * In the following, u refers to variables belonging to the first set and are encoded by idx*2 as
 * even numbers, and v to the second state encoded by idx*2 + 1 as odd numbers.
 */

#include "headers/bdd.hpp"



/**
 * A class that stores info regarding state space, including size of bit vector representation of a
 * state
 */
class StateSpace
{
    public:
        /**
         * Create a new context with `state_bits` bits representing a state
         */
        StateSpace(int state_bits);

    private:
        int state_bits;
        BDD var_eq_bdd;

    friend class Predicate;
    friend class Transition;
};



/** 
 * A class to represent a transition relation as a predicate on pairs of states. We simultaneously
 * maintain two BDD representations of the transition, one for transition from a predicate expressed
 * in the first set of vars to one in the second set, and another for the other way around.
 */
class Transition
{
    public:
        /**
         * Constructor to create a new Transition from a from a variable index. The boolean `to_var`
         * represents weather the given variable index refers to a variable of the state the
         * transition is from (corresponding to the value `false`) or if it belongs to the state the
         * transition is true (value `true`).
         */
        Transition(int var_idx, bool to_var);

        /**
         * Constructor to make a predicate representing full or empty state space depending on value
         * of bconst
         */
        Transition(bool bconst);
        
        /**
         * Copy constructor and assignment operator
         */
        Transition(const Transition& other);
        Transition& operator=(const Transition& other);

        /** 
         * Logical operators for Transition
         */
        friend Transition operator&&(const Transition& bddl, const Transition& bddr);
        friend Transition operator||(const Transition& bddl, const Transition& bddr);
        friend Transition operator^ (const Transition& bddl, const Transition& bddr);
        Transition& operator&=(const Transition& other);
        Transition& operator|=(const Transition& other);
        Transition& operator^=(const Transition& other);
        Transition  operator! ();

        
    private:
        Transition(const BDD& tuv, const BDD& tvu);
        
        BDD t_u_v;           // Repr for var -> var2
        BDD t_v_u;          // Repr for var2 -> var
};



/**
 * A class to represent a predicate on states. Can be combined with algebraic operations.
 */
class Predicate
{
    public:
        /**
         * Constructor to create a new pred from a from a variable index and context.
         */
        Predicate(const StateSpace& sp, int var_idx);

        /**
         * Constructor to make a predicate representing full or empty state space depending on value
         * of bconst
         */
        Predicate(const StateSpace& sp, bool bconst);
        
        /** 
         * Logical operators for Predicate
         */
        friend Predicate operator&&(const Predicate& predl, const Predicate& predr);
        friend Predicate operator||(const Predicate& predl, const Predicate& predr);
        friend Predicate operator^ (const Predicate& predl, const Predicate& predr);
        Predicate& operator&=(const Predicate& other);
        Predicate& operator|=(const Predicate& other);
        Predicate& operator^=(const Predicate& other);
        Predicate  operator! ();

        /**
         * EX and AX can be viewed as operators on Predicate given a Transition
         */
        Predicate EX(const Transition& trans);
        Predicate AX(const Transition& trans);

        /**
         * Check if two Predicates are equal
         */
        friend bool operator==(const Predicate& bddl, const Predicate& bddr);
        friend bool operator!=(const Predicate& bddl, const Predicate& bddr);

 

    private:
        Predicate(const StateSpace& sp, const BDD& repr, bool is_repr_u);
        void to_u_repr();           // Convert representation to u form

        const StateSpace& space;
        BDD p_u;                    // Representation as fn of var
        BDD p_v;                    // Representation as fn of var2
        bool is_p_u_repr;           // Is the correct representation on var u
};
