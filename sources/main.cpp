/**
 * The main file, compliles to an executable
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <exception>

#include "cudd.h"
#define SEXPRESSO_OPT_OUT_PIKESTYLE
#include "sexpresso/sexpresso.hpp"

#include "headers/bdd.hpp"
#include "headers/pred.hpp"


////int main(int argc, char** argv)
////{
////    /*BDD dd1(false);
////    BDD dd2(0);
////    dd1 |= dd2;
////    dd1.save_dot("./out/dbg.dot");*/
////
////    DdManager* man = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
////    DdNode* dd1 = Cudd_ReadLogicZero(man);
////    Cudd_Ref(dd1);
////    DdNode* dd2 = Cudd_bddIthVar(man, 0);
////    Cudd_Ref(dd2);
////    DdNode* dd3 = Cudd_bddOr(man, dd1, dd2);
////    Cudd_Ref(dd3);
////}
//
//
///*
// * For now, the main function simply computes EX, EXEX and AX for the transition function for a mod 2
// * counter, and checks EXp = !p, EXEXp = p
// */
//int main(int argc, char** argv)
//{
//    // We define our state space to be over one bit
//    StateSpace sp(1);
//
//    // We firstly define p, q as predicates over sp
//    predicate p = !predicate(sp, 0);                                            // p = v0<=>0 = !v0
//    predicate q = !p;                                                           // q = !p
//
//    // then we define the transition relation over sp
//    transition trans = transition(sp, 0, false) ^ transition(sp, 0, true);      // t(u, v) := u0^v0 
//
//    // now, we get the predicates for exp, axp and exexp.
//    predicate exp = trans.ex(p);
//    predicate axp = trans.ax(p);
//    predicate exexp = trans.ex(trans.ex(p));
//    
//    // print out all bdds generated into dot files
//    p.get_bdd().save_dot("./out/p.dot");
//    q.get_bdd().save_dot("./out/q.dot");
//    exp.get_bdd().save_dot("./out/exp.dot");
//    axp.get_bdd().save_dot("./out/axp.dot");
//    exexp.get_bdd().save_dot("./out/exexp.dot");
//
//    // now we check if exp <=> q and exexp <=> p. as exp(v0) vs some other vi is just the same
//    // function with different names for bound variables, we can just check exp(v0) <=> q(v0) and so
//    // on
//    std::cout << "exp <=> q "       << (exp == q   ? "holds" : "does not hold") << std::endl;
//    std::cout << "ex(exp) <=> p "   << (exexp == p ? "holds" : "does not hold") << std::endl;
//
//    // check some very basic properties
//    predicate efp = trans.ef(p);
//    predicate egp = trans.eg(p);
//    efp.get_bdd().save_dot("./out/efp.dot");
//    egp.get_bdd().save_dot("./out/gfp.dot");
//    (p || trans.ex(p)).get_bdd().save_dot("./out/dbg.dot");
//    trans.ax(p || trans.ex(p)).get_bdd().save_dot("./out/dbg2.dot");
//
//    std::cout << "efp " << (trans.ef(p).is_false() ? "is unsat" : "is sat") << std::endl;
//    std::cout << "egp " << (trans.eg(p).is_false() ? "is unsat" : "is sat") << std::endl;
//    std::cout << "afp " << (trans.af(p).is_false() ? "is unsat" : "is sat") << std::endl;
//    std::cout << "ag(!p => ex(p)) " << (trans.ag(p || trans.ex(p)).is_true() ? "is valid" : "is invalid") << std::endl;
//}




/**
 * print a short message explaining the command line usage
 */
void print_usage()
{
    std::cout <<   "usage: cudd-ctl-mc <spec_path>"                                 << std::endl;
    std::cout <<   "where:"                                                         << std::endl;
    std::cout <<   "    spec_path       -   the path to model and property"         << std::endl;
    std::cout <<   "                        specification"                          << std::endl;
}



/**
 * we firstly parse the command line arguements into the following parameters
 *
 * spec_path        -   a string with the path to the filename from where to load the model and
 *                      property specifications.
 *
 * returns true if arguments were successfully parsed. else, prints out command line usage and
 * returns false.
 */
bool parse_args(int argc, char** argv, std::string& spec_path)
{
    if(argc < 2) { print_usage(); return false; }
    spec_path = argv[1];
    return true;
}


/**
 * parse the given s-expr into a predicate. throws a string error on failure.
 */
Predicate parse_predicate(const StateSpace& sp, const sexpresso::Sexp& expr)
{
    if(expr.isString())
    {
        std::cout << "Predicate must be an s-expression" << std::endl;
        throw std::runtime_error(expr.toString());
    }
    const std::string& fn = expr.value.sexp[0].value.str;
    if(fn == std::string("or"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "Or takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Predicate ret(sp, false);
        for(size_t i = 1; i < expr.childCount(); i++) ret |= parse_predicate(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("and"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "And takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Predicate ret(sp, true);
        for(size_t i = 1; i < expr.childCount(); i++) ret &= parse_predicate(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("xor"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "Xor takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Predicate ret(sp, true);
        for(size_t i = 1; i < expr.childCount(); i++) ret ^= parse_predicate(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("not"))
    {
        if(expr.childCount() != 2)
        {
            std::cout << "Not takes exactly one argument" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        return !parse_predicate(sp, expr.value.sexp[1]);
    }
    else if(fn == std::string("var"))
    {
        if(!(expr.childCount() == 2 && expr.value.sexp[1].isString()))
        {
            std::cout << "variable expression must be of form (var <index>)" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        int var_index;
        try { var_index = std::stoi(expr.value.sexp[1].value.str); }
        catch(const std::invalid_argument& e)
        {
            std::cout << "variable index must be an int" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        catch(const std::out_of_range& e)
        {
            std::cout << "variable index must be less than size of bit vector" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        if(var_index >= sp.state_bits)
        {
            std::cout << "variable index must be less than size of bit vector" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        return Predicate(sp, var_index);
    }
    else
    {
        std::cout << "function in initial state predicate must be or, and, xor, var" << std::endl;
        throw std::runtime_error(expr.toString());
    }
}



/**
 * parse the given s-expr into a transition. throws a string error on failure.
 */
Transition parse_transition(const StateSpace& sp, const sexpresso::Sexp& expr)
{
    if(expr.isString())
    {
        std::cout << "Transition must be an s-expression" << std::endl;
        throw std::runtime_error(expr.toString());
    }
    const std::string& fn = expr.value.sexp[0].value.str;
    if(fn == std::string("or"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "Or takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Transition ret(sp, false);
        for(size_t i = 1; i < expr.childCount(); i++) ret |= parse_transition(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("and"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "And takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Transition ret(sp, true);
        for(size_t i = 1; i < expr.childCount(); i++) ret &= parse_transition(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("xor"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "Xor takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Transition ret(sp, true);
        for(size_t i = 1; i < expr.childCount(); i++) ret ^= parse_transition(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("not"))
    {
        if(expr.childCount() != 2)
        {
            std::cout << "Not takes exactly one argument" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        return !parse_transition(sp, expr.value.sexp[1]);
    }
    else if(fn == std::string("var"))
    {
        if(!(expr.childCount() == 3 && expr.value.sexp[1].isString() && expr.value.sexp[2].isString()))
        {
            std::cout << "Variable expression must be of form (var <var_type> <index>)" << std::endl;
            throw std::runtime_error("Variable expr malformed");
        }
        int var_index;
        try { var_index = std::stoi(expr.value.sexp[2].value.str); }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Variable index must be an int" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        catch(const std::out_of_range& e)
        {
            std::cout << "Variable index must be less than size of bit vector" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        if(var_index >= sp.state_bits)
        {
            std::cout << "Variable index must be less than size of bit vector" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        if( expr.value.sexp[1].value.str != std::string("from") &&
            expr.value.sexp[1].value.str != std::string("to")   )
        {
            std::cout << "Variable type for transition must be to or from" << std::endl;
            throw std::runtime_error(expr.toString());
        } 
        return Transition(sp, var_index, expr.value.sexp[1].value.str == std::string("to"));
    }
    else 
    { 
        std::cout << "Function in transition must be or, and, xor, var" << std::endl;
        throw std::runtime_error(expr.toString());
    }
}



/**
 * Given a property this function checks if the s-expression is of correct syntax
 */
bool check_property(const StateSpace& sp, const sexpresso::Sexp& expr)
{
    if(expr.isString())
    {
        std::cout << "Property must be an s-expression" << std::endl;
        return false;
    }
    const std::string& fn = expr.value.sexp[0].value.str;
    if(     fn == std::string("or")     || 
            fn == std::string("and")    ||
            fn == std::string("xor")    )
    {
        if(expr.childCount() < 3)
        {
            std::cout << fn << " takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        bool ret = true;
        for(size_t i = 1; i < expr.childCount(); i++) ret &= check_property(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("EX")     || 
            fn == std::string("EF")     ||
            fn == std::string("EG")     ||
            fn == std::string("AX")     ||
            fn == std::string("AF")     ||
            fn == std::string("AG")     )
    {
        if(expr.childCount() != 2)
        {
            std::cout << fn << " takes exactly one argument" <<
                std::endl;
            return false;
        }
        return check_property(sp, expr.value.sexp[1]);
    }
    else if(fn == std::string("EU")     || 
            fn == std::string("ER")     ||
            fn == std::string("AU")     ||
            fn == std::string("AR")     )
    {
        if(expr.childCount() != 3)
        {
            std::cout << expr.value.sexp[0].value.str << " takes exactly two arguments" <<
                std::endl;
            return false;
        }
        return check_property(sp, expr.value.sexp[1]) && check_property(sp, expr.value.sexp[2]);
    }
    else if(fn == std::string("var"))
    {
        if(!(expr.childCount() == 2 && expr.value.sexp[1].isString()))
        {
            std::cout << "Variable expression must be of form (var <index>)" << std::endl;
            return false;
        }
        int var_index;
        try { var_index = std::stoi(expr.value.sexp[1].value.str); }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Variable index must be an int" << std::endl;
            return false;
        }
        catch(const std::out_of_range& e)
        {
            std::cout << "Variable index must be less than size of bit vector" << std::endl;
            return false;
        }
        if(var_index >= sp.state_bits)
        {
            std::cout << "Variable index must be less than size of bit vector" << std::endl;
            return false;
        }
        return true;
    }
    else
    {
        std::cout << "Function in property must be or, and, xor, var or a CTL operator" << std::endl;
        return false;
    }
}



/**
 * Converts the CTL expression to a Predicate. Assumes expression to be syntaxially valid
 */
Predicate ctl_to_pred(cosnt StateSpace& sp, const Transition& trans, const sexpresso::Sexp& expr)
{
    const std::string& fn = expr.value.sexp[0].value.str;
    if      (fn == "var")   return Predicate(sp, std::stoi(expr.value.sexp[1].value.str);
    else if (fn == "and")
    {
        Pred ret = ctl_to_pred(sp, expr.value.sexp[1];
        for(int i = 2; i < expr.childCount(); i++) ret &= ctl_to_pred(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "or")
    {
        Pred ret = ctl_to_pred(sp, expr.value.sexp[1];
        for(int i = 1; i < expr.childCount(); i++) ret |= ctl_to_pred(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "xor")
    {
        Pred ret = ctl_to_pred(sp, expr.value.sexp[1];
        for(int i = 1; i < expr.childCount(); i++) ret ^= ctl_to_pred(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "not")   return !ctl_to_pred(trans, expr.value.sexp[1]);
    else if (fn == "EX")    return trans.EX(ctl_to_pred(sp, trans, expr.value.sexp[1]));
    else if (fn == "EF")    return trans.EF(ctl_to_pred(sp, trans, expr.value.sexp[1]));
    else if (fn == "EG")    return trans.EG(ctl_to_pred(sp, trans, expr.value.sexp[1]));
    else if (fn == "EU")    return trans.EU(ctl_to_pred(sp, trans, expr.value.sexp[1]),
                                            ctl_to_pred(sp, trans, expr.value.sexp[2]));
    else if (fn == "ER")    return trans.ER(ctl_to_pred(sp, trans, expr.value.sexp[1]),
                                            ctl_to_pred(sp, trans, expr.value.sexp[2]));
    else if (fn == "AX")    return trans.AX(ctl_to_pred(sp, trans, expr.value.sexp[1]));
    else if (fn == "AF")    return trans.AF(ctl_to_pred(sp, trans, expr.value.sexp[1]));
    else if (fn == "AG")    return trans.AG(ctl_to_pred(sp, trans, expr.value.sexp[1]));
    else if (fn == "AU")    return trans.AU(ctl_to_pred(sp, trans, expr.value.sexp[1]),
                                            ctl_to_pred(sp, trans, expr.value.sexp[2]));
    else if (fn == "AR")    return trans.AR(ctl_to_pred(sp, trans, expr.value.sexp[1]),
                                            ctl_to_pred(sp, trans, expr.value.sexp[2]));
}





/**
 * Main method
 */
int main(int argc, char** argv)
{
    try
    {
        // Read command line arguments
        std::string spec_path;
        if(!parse_args(argc, argv, spec_path)) return EXIT_FAILURE;
        
        std::cout << "Loading specification from file: " << spec_path << std::endl;


        // Read specification file
        std::string spec_str;
        std::ifstream spec_file(spec_path);
        if(!spec_file.is_open())
        {
            std::cout << "Failed to open specification file" << std::endl;
            return EXIT_FAILURE;
        }
        { 
            std::string line; 
            while(std::getline(spec_file, line)) spec_str += line.substr(0, line.find(';'));
        }
        std::cout << "Specification dump \n" << spec_str << std::endl; // DEBUG


        // Parse file and build StateSpace, init Predicate and Transition
        sexpresso::Sexp spec = sexpresso::parse(spec_str);
        if(spec.isString())
        {
            std::cout << "Top level cannot be a string" << std::endl;
            return EXIT_FAILURE;
        }
        spec = spec.value.sexp[0];
        if(!spec.isSexp() || spec.value.sexp[0].value.str != std::string("system")
                          || spec.childCount() != 5) 
        { 
            std::cout << "Top level must be of form (system n_bits init trans props)" << std::endl;
            // DEBUG
            std::cout << (spec.isSexp() ? "Sexp" : "Not Sexp ") << std::endl;
            std::cout << spec.value.sexp[0].toString() << std::endl;
            std::cout << spec.value.sexp[0].childCount() << std::endl;
            return EXIT_FAILURE;
        }
        if(!spec.value.sexp[1].isString())
        {
            std::cout << "First arguement to system is expected to be an int"           << std::endl; 
            std::cout << "representing the bit vector size."                            << std::endl;
            return EXIT_FAILURE;
        }
        int bit_vector_size;
        try { bit_vector_size = std::stoi(spec.value.sexp[1].value.str); }
        catch(const std::invalid_argument& e)
        {
            std::cout << "First arguement to system is expected to be an int"           << std::endl; 
            std::cout << "representing the bit vector size."                            << std::endl;
            return EXIT_FAILURE;
        }
        catch(const std::out_of_range& e)
        {
            std::cout << "State space requires too many bit vectors to represent"       << std::endl;
            return EXIT_FAILURE;
        }
        StateSpace space(bit_vector_size);
        Predicate init = parse_predicate(space, spec.value.sexp[2]);
        Transition trans = parse_transition(space, spec.value.sexp[3]);


        // Check syntax of the properties
        if(!spec.value.sexp[4].isSexp() 
                || !spec.value.sexp[4].value.sexp[0].isString()
                || spec.value.sexp[4].value.sexp[0].value.str != std::string("properties"))
        {
            std::cout << "Fourth arguement to system should be (properties <prop1>)" << std::endl;
            return EXIT_FAILURE;
        }
        for(size_t i = 1; i < spec.value.sexp[4].childCount(); ++i)
            if(!check_property(space, spec.value.sexp[4].value.sexp[i])) return EXIT_FAILURE;
        std::cout << "Specification parsed, syntax is correct" << std::endl;

        
        // Do the basic model checking
        for(size_t i = 1; i < spec.value.sexp[4].childCount(); ++i)
            std::cout << "Property " << i << " is " <<
                ((ctl_to_pred(space, trans, spec.value.sexp[4].value.sexp[i]) & init).is_false() ?
                    "sat" : "unsat") << std::endl;

        
        


        return EXIT_SUCCESS;
    }
    catch(const std::exception& e)
    {
        std::cout << "Terminating due to exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
} 
