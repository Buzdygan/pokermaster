CXX = g++ -g -O3

all: Judge

DummyPlayer.o : Player.o DummyPlayer.cpp
HumanPlayer.o : Player.o HumanPlayer.cpp
Poker.o : Player.o Poker.cpp

Judge: Player.o Utils.o Poker.o DummyPlayer.o HumanPlayer.o Judge.cpp 

clean:
	rm *.o
