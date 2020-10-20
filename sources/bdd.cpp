/**
 * This file implements the BDD class declared in `headers/bdd.hpp`.
 */

#include "headers/bdd.h"

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


BDD::BDD(DdManager* mngr, int var_index) : manager(mngr)
{
    node = Cudd_bddIthVar(manager, var_index);
    if(!node) throw std::runtime_error("DdNode for variable returned null");
    Cudd_Ref(node);
}

BDD::BDD(DdManager* mngr, DdNode* nd) : node(nd), manager(mngr)
{
    Cudd_Ref(node);
}

BDD::BDD(const BDD& other) : node(other.node), manager(other.manager)
{
    Cudd_Ref(node);
}

BDD::~BDD()
{
    Cudd_RecursiveDeref(manager, node);
}

// Wrappers
BDD operator&&(const BDD& bddl, const BDD& bddr)
{
    DdNode* node = Cudd_bddAnd(bddl.manager, bddl.node, bddr.node);
    if(!node) throw std::runtime_error("And of DdNodes returned null");
    return BDD(bddl.manager, node);
}
BDD operator||(const BDD& bddl, const BDD& bddr)
{
    DdNode* node = Cudd_bddOr(bddl.manager, bddl.node, bddr.node);
    if(!node) throw std::runtime_error("And of DdNodes returned null");
    return BDD(bddl.manager, node);
}
BDD operator^(const BDD& bddl, const BDD& bddr)
{
    DdNode* node = Cudd_bddXor(bddl.manager, bddl.node, bddr.node);
    if(!node) throw std::runtime_error("And of DdNodes returned null");
    return BDD(bddl.manager, node);
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
   return BDD(manager, Cudd_Not(node));
}

// DdNode for a given BDD function is guaranteed to be unique
bool operator==(const BDD& bddl, const BDD& bddr) { return bddl.node == bddr.node; }
bool operator!=(const BDD& bddl, const BDD& bddr) { return bddl.node != bddr.node; }
bool BDD::is_zero() { return node == Cudd_ReadZero(manager); }
bool BDD::is_one()  { return node == Cudd_ReadOne(manager);  }

// More wrappers
BDD BDD::existential_abstraction(int var_index)
{
    return BDD(manager, Cudd_bddExistAbstract(manager, node, Cudd_bddIthVar(manager, var_index)));
}
BDD BDD::universal_abstraction(int var_index)
{
    return BDD(manager, Cudd_bddUnivAbstract(manager, node, Cudd_bddIthVar(manager, var_index)));
}

void BDD::save_dot(const std::string& filename)
{
    // Must use c-style files for compatibility
    FILE* file = fopen(filename.c_str(), "w");
    if(!file) throw std::runtime_error("Could not open file " + filename);
    Cudd_DumpDot(manager, 1, &node, NULL, NULL, file);
    if(fclose(file) != 0) throw std::runtime_error("Could not safely close file " + filename); 
} 
