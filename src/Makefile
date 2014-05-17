CXX = g++ -g -O3

all: Judge Tests

CfrPlayer.o : Player.o CfrPlayer.cpp
DummyPlayer.o : Player.o DummyPlayer.cpp
HumanPlayer.o : Player.o HumanPlayer.cpp
SimplePoker.o : SimplePoker.cpp
HoldemPoker.o : HoldemPoker.cpp
HoldemPokerAbstraction.o : HoldemPokerAbstraction.cpp
BasketManager.o : BasketManager.cpp
Cfr.o : Cfr.cpp GameAbstraction.o

Judge: Player.o GameAbstraction.o Utils.o BasketManager.o SimplePoker.o HoldemPoker.o HoldemPokerAbstraction.o CfrPlayer.o DummyPlayer.o HumanPlayer.o Cfr.o Judge.cpp 
Tests: Player.o GameAbstraction.o Utils.o BasketManager.o SimplePoker.o HoldemPoker.o HoldemPokerAbstraction.o CfrPlayer.o DummyPlayer.o HumanPlayer.o Cfr.o Tests.cpp 

clean:
	rm *.o

clean_strategy:
	rm *.stg
