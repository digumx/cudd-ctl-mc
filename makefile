CUDD_FLAGS = -lcudd
SEXPR_FLAGS = -I./extlib/sexpresso/ -L./extlib/sexpresso -lsexpresso
CXX_FLAGS = -I./ --std=c++11 -Wall -Werror
CXX_SRC = sources/bdd.cpp sources/main.cpp sources/pred.cpp
CXX_HDR = headers/bdd.hpp headers/pred.hpp

all: $(CXX_SRC) $(CXX_HDR) sexpresso
	$(CXX) $(CXX_SRC) $(SEXPR_FLAGS) $(CUDD_FLAGS) -o build/cudd-ctl-mc $(CXX_FLAGS) -Wfatal-errors

debug: $(CXX_SRC) $(CXX_HDR) sexpresso
	$(CXX) $(CXX_SRC) $(CUDD_FLAGS) $(SEXPR_FLAGS) -g -o build/debug $(CXX_FLAGS) -Wfatal-errors

debug-all-errors: $(CXX_SRC) $(CXX_HDR) sexpresso
	$(CXX) $(CXX_SRC) $(CUDD_FLAGS) $(SEXPR_FLAGS) -g -o build/debug $(CXX_FLAGS)

sexpresso: extlib/sexpresso/sexpresso/sexpresso.cpp extlib/sexpresso/sexpresso/sexpresso.hpp
	$(CXX) -o extlib/sexpresso/sexpresso.o -pedantic -Wno-return-type -O3 --std=c++11 -I./extlib/sexpresso/sexpresso -c ./extlib/sexpresso/sexpresso/sexpresso.cpp
	ar rcvs extlib/sexpresso/libsexpresso.a extlib/sexpresso/sexpresso.o
