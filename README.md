This is a very simple CTL model checker which uses BDDs to represent predicates. It uses the 
[CUDD](https://davidkebo.com/cudd) backend. This was made as a part of an assignment for the
MCSV2020 course at CMI.

This is hosted at [github](https://github.com/digumx/cudd-ctl-mc).


# Dependencies:

The project depends on Cudd being installed in the system. The headers for Cudd should be available
in the standard include search path, and the library binary should be available in the linker search
path for building and running to succeed.

The project also uses the [sexpresso](https://github.com/BitPuffin/sexpresso) library for parsing
s-expressions. This is included as a git submodule.


# Building and Running:

Once the project has been cloned locally and the submodules updated, just run make with the given
makefile. Use `make debug` to build with debugging symbols. The makefile was tested with GNU Make
4.3 under gcc 10.2.0. The binary is generated in the `build` directory as `cudd-ctl-mc`.

To run the program, execute the binary with a single argument refering to the specificaton file to
check. The output will describe which of the properties specified are satisfiable or unsatisfiable,
and print out witness or counterexample paths when possible.


# Specification File Syntax:

The specification files are written in a lisp like syntax. Any text between a `;` and the newline
character is treated as a comment and ignored. Apart from that, the syntax is given as follows:

At the topmost level, everything is enclosed in the `system` "function" call. This takes 4 or 5
arguments, described as follows:

1. The first argument is an integer representing the number of bits used to represent state.
2. The second is a propositional formula representing the initial states
3. The third is a propositional formula representing the transition relations.
4. The fourth is the property specification.
5. The fifth is an optional list of fairness constraints.

Thus, the overall high level syntax looks like: 

```lisp
(system <n_bits> <init> <trans> <spec> [fairness])
```

Each of the individual arguments are described further below:

## Initial States:

The initial states are specified as a propositional formula over the bits representing the states.
The formula is written in a lisp like prefix style, with each operand acting like a function and the
joining of `expr1` and `expr2` by `operand` looks like `(operand expr1 expr2)`.
The grammar is given below:

formula f = | false, true           - Boolean constants
            | (var k)               - Represents the k'th bit in the state's bit vector. k < n_bits.
            | (and f1 f2.. )        - Conjunction of f1, f2..
            | (or f1 f2..  )        - Disjunction of f1, f2..
            | (xor f1 f2.. )        - Exclusive or of f1, f2..
            | (not f)               - Negation of f

So, `(or (var 0) (not (var 1)))` represents an initial state where iether the first bit is 1 or the
second bit is 0.

## Transition Relation:

The synatax for the transition relation is a similar propositional formula written as funtion
applications in a prefix style. In fact, the syntax is exactly the same as that of the initial
state, except for variables. The transition relation is a boolean function on a pair of states, and
thus involves two sets of variables, one for the state in which the transition starts, and another
for the state where it ends. These are denoted by `(var from k)`, and `(var to k)` for the variables
representing the kth bit of the state from which the transition starts and to which the transition
goes, respectively. Thus, the grammar is:

transition t =  | false, true           - Boolean constants
                | (var from k)          - Represents the k'th bit in the starting state's bit vector. k < n_bits.
                | (var to   k)          - Represents the k'th bit in the ending state's bit vector. k < n_bits.
                | (and t1 t2.. )        - Conjunction of t1, t2..
                | (or t1 t2..  )        - Disjunction of t1, t2..
                | (xor t1 t2.. )        - Exclusive or of t1, t2..
                | (not t)               - Negation of t

So, a transition relation that just flips the 0th bit of all states is given as 
`(xor (var to 0) (var from 0))`.

## Properties:

The properies are specified as the list of arguments to a `properties` function. Thus, the `<spec>`
specification looks like `(properties <prop1> <prop2>...)`. Each of the given properties are checked
individually with respect to all of the provided fairness constraints.

Each property `<prop>` is a CTL formula, specified in the prefix function application style. The
varables are specified like in the initial states formulae, and have the same interpretation. The
grammar is given as:

ctl f = | false, true           - Boolean constants
        | (var from k)          - Represents the k'th bit in the starting state's bit vector. k < n_bits.
        | (var to   k)          - Represents the k'th bit in the ending state's bit vector. k < n_bits.
        | (and f1 f2.. )        - Conjunction of f1, f2..
        | (or f1 f2..  )        - Disjunction of f1, f2..
        | (xor f1 f2.. )        - Exclusive or of f1, f2..
        | (not f)               - Negation of f
        | (EX f)                - CTL quantifications of f
        | (EF f)
        | (EG f)
        | (EU f1 f2)
        | (ER f1 f2)
        | (AX f)                
        | (AF f)
        | (AG f)
        | (AU f1 f2)
        | (AR f1 f2)


Thus if we wanted to check two properties, say if there is a path from initial states to a state
where v0 holds, and if all paths lead to a state where v0 holds, then the spec section would look
like `(properties (EF (var 0)) (AF (var 0)))`.

# Fairness:

Fairness is given as a list of propositional formulae each of which are specified exactly in the
same way as the initial states were: `(<cond1> <cond2> ...)`. A path is fair iff each of these
conditions becomes true infinitely often in the path. The grammar for the `<cond>` is the same as
that of the initial state specification.
