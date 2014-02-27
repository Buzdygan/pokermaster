CXX = g++ -g -O3

all: Judge Tests

CfrPlayer.o : Player.o CfrPlayer.cpp
DummyPlayer.o : Player.o DummyPlayer.cpp
HumanPlayer.o : Player.o HumanPlayer.cpp
SimplePoker.o : SimplePoker.cpp
Cfr.o : Cfr.cpp GameAbstraction.o

Judge: Player.o GameAbstraction.o Utils.o SimplePoker.o CfrPlayer.o DummyPlayer.o HumanPlayer.o Cfr.o Judge.cpp 
Tests: Player.o GameAbstraction.o Utils.o SimplePoker.o CfrPlayer.o DummyPlayer.o HumanPlayer.o Cfr.o Tests.cpp 

clean:
	rm *.o
