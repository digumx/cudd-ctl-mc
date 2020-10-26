CXX_OPTS = -I./ -l cudd -Wfatal-errors
CXX_SRC = sources/bdd.cpp sources/main.cpp

all:
	$(CXX) -o build/cudd-ctl-mc $(CXX_OPTS) $(CXX_SRC)

debug: 
	$(CXX) -g -o build/debug $(CXX_OPTS) $(CXX_SRC)
