CUDD_FLAGS = -l cudd
CXX_FLAGS = -I./ -Wfatal-errors --std=c++11
CXX_SRC = sources/bdd.cpp sources/main.cpp

all:
	$(CXX) -o build/cudd-ctl-mc $(CUDD_FLAGS) $(CXX_FLAGS) $(CXX_SRC)

debug: 
	$(CXX) -g -o build/debug $(CUDD_FLAGS) $(CXX_FLAGS) $(CXX_SRC)
