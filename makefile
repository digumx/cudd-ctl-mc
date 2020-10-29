CUDD_FLAGS = -l cudd
CXX_FLAGS = -I./ --std=c++11
CXX_ERR_FLAGS = -Wfatal-errors
CXX_SRC = sources/bdd.cpp sources/main.cpp sources/pred.cpp

all:
	$(CXX) -o build/cudd-ctl-mc $(CUDD_FLAGS) $(CXX_FLAGS) $(CXX_ERR_FLAGS) $(CXX_SRC)

debug: 
	$(CXX) -g -o build/debug $(CUDD_FLAGS) $(CXX_FLAGS) $(CXX_ERR_FLAGS) $(CXX_SRC)

debug-all-errors:
	$(CXX) -g -o build/debug $(CUDD_FLAGS) $(CXX_FLAGS) $(CXX_SRC)
