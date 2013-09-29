CXX = g++ -g -O3

all: Judge

DummyPlayer.o : Player.o DummyPlayer.cpp
Poker.o : Player.o Poker.cpp

Judge: Player.o Utils.o Poker.o DummyPlayer.o Judge.cpp 

clean:
	rm *.o
