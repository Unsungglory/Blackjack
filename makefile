# cse3310 project sample makefile



CXX=g++
# note the below asio package may be in a 
# different place
CPPFLAGS=-I../asio-1.13.0/include
CXXFLAGS=-Wall -g -std=c++11 

TARGETS= player dealer

all:${TARGETS}


# these compile and link in the same command.
# no intermediate object code files

player: player.cpp
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ $< -lpthread

dealer: dealer.cpp
	${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ $< -lpthread

clean:
	-rm -f ${TARGETS}
