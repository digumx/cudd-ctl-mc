/**
 * Header file declaring a class to represent a BDD. This will be a wrapper around the CUDD
 * interface
 */

#ifndef BDD_H
#define BDD_H

#include <string>                   // Filenames
#include <vector>                   // List of variables

/** 
 * Forward declare DdNode and DdManager here
 */
struct DdNode;
struct DdManager;


/**
 * Class representing a BDD. Wraps around DdNode, and takes care of reference incrementing and
 * decrementing. Also provides nice syntax for logical operations on BDD.
 */
class BDD
{
    public:
        /**
         * Constructor to create a new BDD representing just the variable with the given index. Must
         * be given a manager.
         */
        BDD(int var_index);

        /**
         * Construct a BDD representing the constant function given by `bconst`
         */
        BDD(bool bconst);

        /**
         * Construct a cube BDD from a given vector of variables
         */
        BDD(std::vector<int> var_indices);
        
        /**
         * Copy constructor
         */
        BDD(const BDD& other);

        /**
         * Copy assignment operator
         */
        BDD& operator=(const BDD& other);

        /**
         * Destructor
         */
        ~BDD();

        /**
         * Logical operators, along with their assignment counterparts. Note that calling these is
         * safe only if both operands are initialized with same manager.
         */
        friend BDD operator&&(const BDD& bddl, const BDD& bddr);
        friend BDD operator||(const BDD& bddl, const BDD& bddr);
        friend BDD operator^ (const BDD& bddl, const BDD& bddr);
        BDD& operator&=(const BDD& other);
        BDD& operator|=(const BDD& other);
        BDD& operator^=(const BDD& other);
        BDD  operator! ();

        /**
         * Check if two BDDs are equal
         */
        friend bool operator==(const BDD& bddl, const BDD& bddr);
        friend bool operator!=(const BDD& bddl, const BDD& bddr);

        /**
         * Check if the BDD represents constant zero or constant one. This can be used to check
         * satisfiability or validity
         */
        bool is_zero(); 
        bool is_one();

        /**
         * Abstract the BDD using quantifiers over variable with given index, or vector of variables
         * with given indices, or over variables in the set represented by a cube.
         */
        BDD existential_abstraction(int var_index); 
        BDD existential_abstraction(std::vector<int> var_indices); 
        BDD existential_abstraction(const BDD& cube); 
        BDD universal_abstraction  (int var_index); 
        BDD universal_abstraction  (std::vector<int> var_indices); 
        BDD universal_abstraction  (const BDD& cube); 

        /**
         * Print out a representation of the BDD in dot format
         */
        void save_dot(const std::string& filename, bool draw_0_arc = false);


    private:
        BDD(DdNode* nd);
        
        DdNode* node;
        static DdManager* manager;
};

#endif
