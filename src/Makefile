.PHONY: all clean

CXX:=g++
CXXFLAGS:=-Wall -Wextra -g -DDEBUG -DHAVE_CONFIG_H
LDFLAGS:=

clean:
	rm -f *.o subscale

all: subscale

subscale: main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.cpp common.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
