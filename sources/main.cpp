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
 * Parse the given s-expr into a predicate. throws a runtime_error containing string representation
 * of problematic s-expr on failure.
 */
Predicate parse_predicate(const StateSpace& sp, const sexpresso::Sexp& expr)
{
    if(expr.isString())
    {
        if(expr.value.str == "true")        return Predicate(sp, true);
        else if(expr.value.str == "false")  return Predicate(sp, false);

        std::cout << "Constant predicate must be true or false" << std::endl;
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
        Predicate ret = parse_predicate(sp, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret |= parse_predicate(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("and"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "And takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Predicate ret = parse_predicate(sp, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret &= parse_predicate(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("xor"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "Xor takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Predicate ret = parse_predicate(sp, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret ^= parse_predicate(sp, expr.value.sexp[i]);
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
        if(expr.value.str == "true")        return Transition(sp, true);
        else if(expr.value.str == "false")  return Transition(sp, false);

        std::cout << "constant in transition must be `true` or `false`" << std::endl;
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
        Transition ret = parse_transition(sp, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret |= parse_transition(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("and"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "And takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Transition ret = parse_transition(sp, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret &= parse_transition(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("xor"))
    {
        if(expr.childCount() < 3)
        {
            std::cout << "Xor takes atleast two arguments" << std::endl;
            throw std::runtime_error(expr.toString());
        }
        Transition ret = parse_transition(sp, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret ^= parse_transition(sp, expr.value.sexp[i]);
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
        if(expr.value.str == "true" || expr.value.str == "false") return true;

        std::cout << "Constant in property must be true or false" << std::endl;
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
            return false;
        }
        bool ret = true;
        for(size_t i = 1; i < expr.childCount(); i++) ret &= check_property(sp, expr.value.sexp[i]);
        return ret;
    }
    else if(fn == std::string("not"))
    {
        if(expr.childCount() != 2)
        {
            std::cout << fn << " takes exacly one argument" << std::endl;
            return false;
        }
        return check_property(sp, expr.value.sexp[1]);
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
Predicate ctl_to_pred(const StateSpace& sp, const Transition& trans, const sexpresso::Sexp& expr)
{
    if(expr.isString())
    {
        if(expr.value.str == "true")        return Predicate(sp, true);
        else if(expr.value.str == "false")  return Predicate(sp, false);

        std::cout << "Constant predicate must be true or false" << std::endl;
        throw std::runtime_error(expr.toString());
    }
    const std::string& fn = expr.value.sexp[0].value.str;
    if      (fn == "var")   return Predicate(sp, std::stoi(expr.value.sexp[1].value.str));
    else if (fn == "and")
    {
        Predicate ret = ctl_to_pred(sp, trans, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret &= ctl_to_pred(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "or")
    {
        Predicate ret = ctl_to_pred(sp, trans, expr.value.sexp[1]);
        for(size_t i = 1; i < expr.childCount(); i++) ret |= ctl_to_pred(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "xor")
    {
        Predicate ret = ctl_to_pred(sp, trans, expr.value.sexp[1]);
        for(size_t i = 1; i < expr.childCount(); i++) ret ^= ctl_to_pred(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "not")   return !ctl_to_pred(sp, trans, expr.value.sexp[1]);
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
    else throw std::runtime_error("Unknown function in property specification");
}

/**
 * Version of previous function modified to handle fairness constraints
 */ 
Predicate ctl_to_pred_fair(const StateSpace& sp, const Transition& trans, const sexpresso::Sexp& expr)
{
    if(expr.isString())
    {
        if(expr.value.str == "true")        return Predicate(sp, true);
        else if(expr.value.str == "false")  return Predicate(sp, false);

        std::cout << "Constant predicate must be true or false" << std::endl;
        throw std::runtime_error(expr.toString());
    }
    const std::string& fn = expr.value.sexp[0].value.str;
    if      (fn == "var")   return Predicate(sp, std::stoi(expr.value.sexp[1].value.str)) &&
                                    trans.EG_fair(Predicate(sp, true));
    else if (fn == "and")
    {
        Predicate ret = ctl_to_pred_fair(sp, trans, expr.value.sexp[1]);
        for(size_t i = 2; i < expr.childCount(); i++) ret &= ctl_to_pred_fair(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "or")
    {
        Predicate ret = ctl_to_pred_fair(sp, trans, expr.value.sexp[1]);
        for(size_t i = 1; i < expr.childCount(); i++) ret |= ctl_to_pred_fair(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "xor")
    {
        Predicate ret = ctl_to_pred_fair(sp, trans, expr.value.sexp[1]);
        for(size_t i = 1; i < expr.childCount(); i++) ret ^= ctl_to_pred_fair(sp, trans, expr.value.sexp[i]);
        return ret;
    }
    else if (fn == "not")   return !ctl_to_pred_fair(sp, trans, expr.value.sexp[1]);
    else if (fn == "EX")    return trans.EX_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]));
    else if (fn == "EF")    return trans.EF_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]));
    else if (fn == "EG")    return trans.EG_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]));
    else if (fn == "EU")    return trans.EU_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]),
                                                 ctl_to_pred_fair(sp, trans, expr.value.sexp[2]));
    else if (fn == "ER")    return trans.ER_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]),
                                                 ctl_to_pred_fair(sp, trans, expr.value.sexp[2]));
    else if (fn == "AX")    return trans.AX_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]));
    else if (fn == "AF")    return trans.AF_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]));
    else if (fn == "AG")    return trans.AG_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]));
    else if (fn == "AU")    return trans.AU_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]),
                                                 ctl_to_pred_fair(sp, trans, expr.value.sexp[2]));
    else if (fn == "AR")    return trans.AR_fair(ctl_to_pred_fair(sp, trans, expr.value.sexp[1]),
                                                 ctl_to_pred_fair(sp, trans, expr.value.sexp[2]));
    else throw std::runtime_error("Unknown function in property specification");
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


        // Parse file and build StateSpace, init Predicate and Transition
        sexpresso::Sexp spec = sexpresso::parse(spec_str);
        if(spec.isString())
        {
            std::cout << "Top level cannot be a string" << std::endl;
            return EXIT_FAILURE;
        }
        spec = spec.value.sexp[0];
        if(!spec.isSexp() || spec.value.sexp[0].value.str != std::string("system")
                          || (spec.childCount() != 5 && spec.childCount() != 6)) 
        { 
            std::cout << "Top level must be of form (system n_bits init trans props [fairness])" << std::endl;
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


        // Check if fairness conditions are provided
        if(spec.childCount() == 6)
        {
            std::cout << "Reading fairness conditions" << std::endl;
            // Parse the fairness conditions
            if(spec.value.sexp[5].isString())
            {
                std::cout << "Fifth argument should be a list of fairness conditions (f1 f2..)" <<
                    std::endl;
                return EXIT_FAILURE;
            }
            for(size_t i = 0; i < spec.value.sexp[5].childCount(); ++i)
               trans.add_fairness(parse_predicate(space, spec.value.sexp[5].value.sexp[i]));


            // Do the model checking
            for(size_t i = 1; i < spec.value.sexp[4].childCount(); ++i)
                std::cout << "Property " << i << " is " <<
                    ((ctl_to_pred_fair(space, trans, spec.value.sexp[4].value.sexp[i]) || !init).is_true() ?
                        "sat" : "unsat") << std::endl;


            return EXIT_SUCCESS;
        }

        
        // Loop over all properties again and model check them
        for(size_t i = 1; i < spec.value.sexp[4].childCount(); ++i)
        {
            const sexpresso::Sexp& prop = spec.value.sexp[4].value.sexp[i];
            if(prop.isString())
            {
                std::cout << "Property " << i << " is " <<
                    ((ctl_to_pred(space, trans, prop) || !init).is_true() ? "sat" : "unsat") << 
                    std::endl;
                std::cout << "Could not generate witness or counterexample. " << std::endl;

            }
            const std::string& fn = prop.value.sexp[0].value.str;
            // Case split over each outer level connective to handle each connective differently for
            // cex/witness generation
            if(fn == "EF")
            {
                const sexpresso::Sexp& subprop = prop.value.sexp[1];
                Predicate subpred = ctl_to_pred(space, trans, subprop);
                Predicate pred = trans.EF(subpred);
                if((pred || !init).is_true())
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Witness: " << std::endl;
                    trans.gen_witness_EF(init, pred, subpred).print();
                }
                else
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Cannot generate counterexample for EF" << std::endl;
                }
            }
            else if(fn == "EG")
            {
                const sexpresso::Sexp& subprop = prop.value.sexp[1];
                Predicate subpred = ctl_to_pred(space, trans, subprop);
                Predicate pred = trans.EG(subpred);
                if((pred || !init).is_true())
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Witness: " << std::endl;
                    trans.gen_witness_EG(init, pred, subpred).print();
                }
                else
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Cannot generate counterexample for EG" << std::endl;
                }
            }    
            else if(fn == "EU")
            {
                const sexpresso::Sexp& subpropl = prop.value.sexp[1];
                const sexpresso::Sexp& subpropr = prop.value.sexp[2];
                Predicate subpredl = ctl_to_pred(space, trans, subpropl);
                Predicate subpredr = ctl_to_pred(space, trans, subpropr);
                Predicate pred = trans.EU(subpredl, subpredr);
                if((pred || !init).is_true())
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Witness: " << std::endl;
                    trans.gen_witness_EU(init, pred, subpredl, subpredr).print();
                }
                else
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Cannot generate counterexample for EU" << std::endl;
                }
            }
            else if(fn == "ER")
            {
                const sexpresso::Sexp& subpropl = prop.value.sexp[1];
                const sexpresso::Sexp& subpropr = prop.value.sexp[2];
                Predicate subpredl = ctl_to_pred(space, trans, subpropl);
                Predicate subpredr = ctl_to_pred(space, trans, subpropr);
                Predicate pred = trans.ER(subpredl, subpredr);
                if((pred || !init).is_true())
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Witness: " << std::endl;
                    trans.gen_witness_ER(init, pred, subpredl, subpredr).print();
                }
                else
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Cannot generate counterexample for ER" << std::endl;
                }
            }
            else if(fn == "AF")
            {
                const sexpresso::Sexp& subprop = prop.value.sexp[1];
                Predicate subpred = ctl_to_pred(space, trans, subprop);
                Predicate pred = trans.AF(subpred);
                if(!((pred || !init).is_true()))
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Counterexample: " << std::endl;
                    trans.gen_cex_AF(init, pred, subpred).print();
                }
                else
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Cannot generate witness for AF" << std::endl;
                }
            }
            else if(fn == "AG")
            {
                const sexpresso::Sexp& subprop = prop.value.sexp[1];
                Predicate subpred = ctl_to_pred(space, trans, subprop);
                Predicate pred = trans.AG(subpred);
                if(!((pred || !init).is_true()))
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Counterexample: " << std::endl;
                    trans.gen_cex_AG(init, pred, subpred).print();
                }
                else
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Cannot generate witness for AG" << std::endl;
                }
            }   
            else if(fn == "AU")
            {
                const sexpresso::Sexp& subpropl = prop.value.sexp[1];
                const sexpresso::Sexp& subpropr = prop.value.sexp[2];
                Predicate subpredl = ctl_to_pred(space, trans, subpropl);
                Predicate subpredr = ctl_to_pred(space, trans, subpropr);
                Predicate pred = trans.AU(subpredl, subpredr);
                if(!((pred || !init).is_true()))
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Counterexample: " << std::endl;
                    trans.gen_cex_AU(init, pred, subpredl, subpredr).print();
                }
                else
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Cannot generate witness for AU" << std::endl;
                }
            }
            else if(fn == "AR")
            {
                const sexpresso::Sexp& subpropl = prop.value.sexp[1];
                const sexpresso::Sexp& subpropr = prop.value.sexp[2];
                Predicate subpredl = ctl_to_pred(space, trans, subpropl);
                Predicate subpredr = ctl_to_pred(space, trans, subpropr);
                Predicate pred = trans.AR(subpredl, subpredr);
                if(!((pred || !init).is_true()))
                {
                    std::cout << "Property " << i << " is unsat." << std::endl;
                    std::cout << "Counterexample: " << std::endl;
                    trans.gen_cex_AR(init, pred, subpredl, subpredr).print();
                }
                else
                {
                    std::cout << "Property " << i << " is sat." << std::endl;
                    std::cout << "Cannot generate witness for AR" << std::endl;
                }
            }
            // If the outermost connective is none of the above, then do the standard MC without
            // counterexample generation.
            else 
            {
                std::cout << "Property " << i << " is " <<
                    ((ctl_to_pred(space, trans, prop) && init).is_false() ? "unsat" : "sat") << 
                    std::endl;
                std::cout << "Could not generate witness or counterexample for top level " << fn <<
                    std::endl;
            } 
        }

        
        


        return EXIT_SUCCESS;
    }
    catch(const std::exception& e)
    {
        std::cout << "Terminating due to exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
} 
