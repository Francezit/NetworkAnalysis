g++ -g -c core/utility.cpp
g++ -g -c core/QQplotGenerator.cpp
g++ -g -c core/shell.cpp

g++ -g -c flowsolver/flowsolverbase.cpp
g++ -g -c flowsolver/acosolver.cpp
g++ -g -c flowsolver/immunesolver.cpp
g++ -g -c flowsolver/ffsolver.cpp

g++ -g -c network/topology.cpp
g++ -g -c network/network.cpp

g++ -g -c main.cpp
g++ -g -o networkanalysis.exe utility.o QQplotGenerator.o shell.o topology.o network.o flowsolverbase.o acosolver.o immunesolver.o ffsolver.o main.o

move main.o bin/debug/main.o

move network.o bin/debug/network.o
move topology.o bin/debug/topology.o

move utility.o bin/debug/utility.o
move QQplotGenerator.o bin/debug/QQplotGenerator.o
move shell.o bin/debug/shell.o

move flowsolverbase.o bin/debug/flowsolverbase.o
move immunesolver.o bin/debug/immunesolver.o
move ffsolver.o bin/debug/ffsolver.o
move acosolver.o bin/debug/acosolver.o

move networkanalysis.exe bin/debug/networkanalysis.exe