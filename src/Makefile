CXX = g++ -g -O3 -mcmodel=medium

all: Judge Tests

CfrPlayer.o : Player.o CfrPlayer.cpp
CfrAbstractionPlayer.o : Player.o CfrAbstractionPlayer.cpp
CfrModPlayer.o : Player.o CfrPlayer.o CfrModPlayer.cpp
DummyPlayer.o : Player.o DummyPlayer.cpp
HumanPlayer.o : Player.o HumanPlayer.cpp
SimplePoker.o : SimplePoker.cpp
HoldemPoker.o : HoldemPoker.cpp
HoldemPokerAbstraction.o : HoldemPokerAbstraction.cpp
HoldemPokerModAbstraction.o : HoldemPokerModAbstraction.cpp
BasketManager.o : BasketManager.cpp
Cfr.o : Cfr.cpp GameAbstraction.o
ModCfr.o : ModCfr.cpp GameAbstraction.o

Judge: Player.o GameAbstraction.o Utils.o BasketManager.o SimplePoker.o HoldemPoker.o HoldemPokerAbstraction.o HoldemPokerModAbstraction.o CfrPlayer.o CfrModPlayer.o CfrAbstractionPlayer.o DummyPlayer.o HumanPlayer.o Cfr.o ModCfr.o Judge.cpp 
Tests: Player.o GameAbstraction.o Utils.o BasketManager.o SimplePoker.o HoldemPoker.o HoldemPokerAbstraction.o HoldemPokerModAbstraction.o CfrPlayer.o CfrModPlayer.o CfrAbstractionPlayer.o DummyPlayer.o HumanPlayer.o Cfr.o ModCfr.o Tests.cpp 

clean:
	rm *.o

clean_strategy:
	rm *.stg
