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

#ifndef PRED_H
#define PRED_H

#include "headers/bdd.hpp"



/** 
 * Forward declarations
 */
class StateSpace;
class Predicate;
class Transition;



/**
 * A class that stores info regarding state space, including size of bit vector representation of a
 * state
 */
class StateSpace
{
    public:
        /**
         * The number of bits in the state space.
         */
        const int state_bits;

        /**
         * Create a new context with `state_bits` bits representing a state
         */
        StateSpace(int state_bits);

        /**
         * Equality operators
         */
        friend bool operator==(const StateSpace& sl, const StateSpace& sr);
        friend bool operator!=(const StateSpace& sl, const StateSpace& sr);

    private:
        BDD var_eq_bdd;
        BDD cube_u;
        BDD cube_v;

    friend class Predicate;
    friend Predicate operator&&(const Predicate& predl, const Predicate& predr);
    friend Predicate operator||(const Predicate& predl, const Predicate& predr);
    friend Predicate operator^ (const Predicate& predl, const Predicate& predr);
    friend bool operator==(const Predicate& predl, const Predicate& predr);
    friend bool operator!=(const Predicate& predl, const Predicate& predr);
    
    friend class Transition;
    friend Transition operator&&(const Transition& trl, const Transition& trr);
    friend Transition operator||(const Transition& trl, const Transition& trr);
    friend Transition operator^ (const Transition& trl, const Transition& trr);
    friend bool operator==(const Transition& trl, const Transition& trr);
    friend bool operator!=(const Transition& trl, const Transition& trr);
        

};



/** 
 * A class to represent a transition relation as a predicate on pairs of states. We simultaneously
 * maintain two BDD representations of the transition, one for transition from a predicate expressed
 * in the first set of vars to one in the second set, and another for the other way around.
 */
class Transition
{
    public:
        /** The state space of the transition. While the implementation does not require a
         * Transition object to have a StateSpace, it is nonetheless added in and required by the
         * constructors to make explicit the fact that a Transition only makes sense with respect to
         * a StateSpace. The StateSpace is only used for sanity checking and throwing errors if the
         * sanity check fails.
         */
        const StateSpace& space;
        /**
         * Constructor to create a new Transition from a from a variable index. The boolean `to_var`
         * represents weather the given variable index refers to a variable of the state the
         * transition is from (corresponding to the value `false`) or if it belongs to the state the
         * transition is true (value `true`).
         */
        Transition(const StateSpace& sp, int var_idx, bool to_var);

        /**
         * Constructor to make a predicate representing full or empty state space depending on value
         * of bconst
         */
        Transition(const StateSpace& sp, bool bconst);

        /** 
         * Copy and assignment
         */
        Transition(const Transition& other);
        Transition& operator=(const Transition& other);

        /**
         * Equality operator
         */
        friend bool operator==(const Transition& trl, const Transition& trr);
        friend bool operator!=(const Transition& trl, const Transition& trr);
        
        /** 
         * Logical operators for Transition
         */
        friend Transition operator&&(const Transition& trl, const Transition& trr);
        friend Transition operator||(const Transition& trl, const Transition& trr);
        friend Transition operator^ (const Transition& trl, const Transition& trr);
        Transition& operator&=(const Transition& other);
        Transition& operator|=(const Transition& other);
        Transition& operator^=(const Transition& other);
        Transition  operator! ();

        /**
         * CTL quantifiers. Based on given transition, convert predicates to predicates representing
         * quantified versions.
         */
        Predicate EX(const Predicate& pred) const;
        Predicate EF(const Predicate& pred) const;
        Predicate EG(const Predicate& pred) const;
        Predicate EU(const Predicate& predl, const Predicate& predr) const;
        Predicate ER(const Predicate& predl, const Predicate& predr) const;
        Predicate AX(const Predicate& pred) const;
        Predicate AF(const Predicate& pred) const;
        Predicate AG(const Predicate& pred) const;
        Predicate AU(const Predicate& predl, const Predicate& predr) const;
        Predicate AR(const Predicate& predl, const Predicate& predr) const;

        
    private:
        Transition(const StateSpace& sp, const BDD& tuv, const BDD& tvu);
        
        BDD t_u_v;          // Repr for var -> var2
        BDD t_v_u;          // Repr for var2 -> var

    friend class Predicate;
};



/**
 * A class to represent a predicate on states. Can be combined with algebraic operations.
 */
class Predicate
{
    public:
        /**
         * The State space the predicate is over
         */
        const StateSpace& space;

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
         * Copy and assignment
         */
        Predicate(const Predicate& other);
        Predicate& operator=(const Predicate& other);
        
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
         * EX and AX can be viewed as operators on Predicate given a Transition over the same state
         * space as predicate.
         */
        Predicate EX(const Transition& trans) const;
        Predicate AX(const Transition& trans) const;

        /**
         * Check if two Predicates are equal
         */
        friend bool operator==(const Predicate& predl, const Predicate& predr);
        friend bool operator!=(const Predicate& predl, const Predicate& predr);

        /**
         * Returns a BDD in u vars
         */
        BDD get_bdd() const;

        /**
         * Get weather the predicate represents the constant true predicate or false predicate
         */
        bool is_true() const;
        bool is_false() const;

 

    private:
        Predicate(const StateSpace& sp, const BDD& repr, bool is_repr_u);
        //void to_u_repr();           // Convert representation to u form

        BDD p_u;                    // Representation as fn of var
        BDD p_v;                    // Representation as fn of var2
        bool is_p_u_repr;           // Is the correct representation on var u

    friend class Transition;
};

#endif
