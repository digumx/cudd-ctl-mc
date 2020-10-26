/**
 * This file defines classes that represent predicates on states represented as boolean functions on
 * bitvectors.
 */

#include "headers/bdd.hpp"

/**
 * A class that stores the necessary context for a predicate, including the bitvector size of the
 * state space the predicate is on
 */
class PredContext
{
    public:
        /**
         * Create a new context with `state_bits` bits representing a state
         */
        PredContext(int state_bits);

    private:
        int state_bits;
        BDD var_eq_bdd;

    friend class Pred;
};

/**
 * A class to represent a predicate on states. Can be combined with algebraic operations.
 */
class Pred
{
    public:
        /**
         * Constructor to create a new pred from a 
};
