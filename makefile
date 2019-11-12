# cse3310 project sample makefile



CXX=g++
# note the below asio package may be in a 
# different place
CPPFLAGS=-I../asio-1.13.0/include
CXXFLAGS=-Wall -g -std=c++11 -O0

# many examples on the web show the 'pkg-config' commands
# being executed during the make process.  this just
# does it once when the makefile is initially parsed


TARGETS= dealer player

all:${TARGETS}

# these compile and link in the same command.
# no intermediate object code files
# (sacrifice compilation speed for not having to
#  deal with dependencies)


player:  player.cpp
	@${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ $< -lpthread 

dealer: dealer.cpp chat_message.hpp
	@${CXX} ${CXXFLAGS} ${CPPFLAGS} -o $@ $< -lpthread

clean:
	-rm -f ${TARGETS} 
