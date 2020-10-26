/**
 * This file defines classes that represent predicates on states represented as boolean functions on
 * bitvectors.
 *
 * To allow predicate transformers to existentially quantify over states and avoid renaming
 * variables, we maintain two representations for the predicate as BDDs over two sets of variables.
 * Existential quantification then converts a BDD over one set of variables to a BDD over another
 * set of variables.
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
         * Constructor to create a new pred from a from a variable index and context.
         */
        Pred(PredContext ctx, int var_idx);

        /**
         * Constructor to make a predicate representing full or empty state space depending on value
         * of bconst
         */
        Pred(PredContext ctx, bool bconst);
        
        /**
         * Copy constructor and assignment operator
         */
        Pred(const Pred& other);
        Pred& operator=(const Pred& other);

    private:
        PredContext& context;
        BDD repr;
        BDD repr2;
        bool is_repr_correct;
};
