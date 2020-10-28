/**
 * This file implements the BDD class declared in `headers/bdd.hpp`.
 */

#include "headers/bdd.hpp"

// For some reason cudd needs declarations in these headers but does not include these within
// cudd.h.....
#include <cstdio>
#include <cstdlib>
extern "C"
{
    #include <sys/types.h>
    #include "cudd.h"
}

#include <stdexcept>


// Initialize static vars, including manager
DdManager* BDD::manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);


// Ctor, dtor and assignment
BDD::BDD(int var_index)
{
    node = Cudd_bddIthVar(BDD::manager, var_index);
    if(!node) throw std::runtime_error("DdNode for variable returned null");
    Cudd_Ref(node);
}

BDD::BDD(bool bconst)
{
    node = bconst ? Cudd_ReadZero(BDD::manager) : Cudd_ReadOne(BDD::manager);
    Cudd_Ref(node);
}

BDD::BDD(DdNode* nd) : node(nd)
{
    Cudd_Ref(node);
}

BDD::BDD(const BDD& other) : node(other.node)
{
    Cudd_Ref(node);
}

BDD& BDD::operator=(const BDD& other)
{
    Cudd_RecursiveDeref(BDD::manager, node);
    node = other.node;
    Cudd_Ref(node);
    return *this;
}

BDD::~BDD()
{
    Cudd_RecursiveDeref(BDD::manager, node);
}


// Wrappers
BDD operator&&(const BDD& bddl, const BDD& bddr)
{
    DdNode* node = Cudd_bddAnd(BDD::manager, bddl.node, bddr.node);
    if(!node) throw std::runtime_error("And of DdNodes returned null");
    return BDD(node);
}
BDD operator||(const BDD& bddl, const BDD& bddr)
{
    DdNode* node = Cudd_bddOr(BDD::manager, bddl.node, bddr.node);
    if(!node) throw std::runtime_error("And of DdNodes returned null");
    return BDD(node);
}
BDD operator^(const BDD& bddl, const BDD& bddr)
{
    DdNode* node = Cudd_bddXor(BDD::manager, bddl.node, bddr.node);
    if(!node) throw std::runtime_error("And of DdNodes returned null");
    return BDD(node);
}

BDD& BDD::operator&=(const BDD& other)
{
    *this = *this && other;
    return *this;
}
BDD& BDD::operator|=(const BDD& other)
{
    *this = *this || other;
    return *this;
}
BDD& BDD::operator^=(const BDD& other)
{
    *this = *this ^ other;
    return *this;
}
BDD BDD::operator!()
{
   return BDD(Cudd_Not(node));
}


// DdNode for a given BDD function is guaranteed to be unique
bool operator==(const BDD& bddl, const BDD& bddr) { return bddl.node == bddr.node; }
bool operator!=(const BDD& bddl, const BDD& bddr) { return bddl.node != bddr.node; }
bool BDD::is_zero() { return node == Cudd_ReadZero(BDD::manager); }
bool BDD::is_one()  { return node == Cudd_ReadOne(BDD::manager);  }


// Wrapper for quantifier eleminations
BDD BDD::existential_abstraction(int var_index)
{
    return BDD(Cudd_bddExistAbstract(BDD::manager, node, Cudd_bddIthVar(BDD::manager, var_index)));
}
BDD BDD::universal_abstraction(int var_index)
{
    return BDD(Cudd_bddUnivAbstract(BDD::manager, node, Cudd_bddIthVar(BDD::manager, var_index)));
}
BDD BDD::existential_abstraction(std::vector<int> var_indices)
{
    return BDD(Cudd_bddExistAbstract(BDD::manager, node, 
                                Cudd_IndicesToCube(BDD::manager, var_indices.data(), var_indices.size())));
}
BDD BDD::universal_abstraction(std::vector<int> var_indices)
{
    return BDD(Cudd_bddUnivAbstract(BDD::manager, node,
                Cudd_IndicesToCube(BDD::manager, var_indices.data(), var_indices.size())));
}


// Utility functions
void BDD::save_dot(const std::string& filename, bool draw_0_arc)
{
    DdNode* nd = draw_0_arc ? node : Cudd_BddToAdd(BDD::manager, node);  // Node to draw
    FILE* file = fopen(filename.c_str(), "w");                      // using c-file for compat
    if(!file) throw std::runtime_error("Could not open file " + filename);
    Cudd_DumpDot(BDD::manager, 1, &nd, NULL, NULL, file);
    if(fclose(file) != 0) throw std::runtime_error("Could not safely close file " + filename); 
} 
