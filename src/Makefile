.PHONY: all clean

CXX:=g++
CXXFLAGS:=-Wall -Wextra -g -DDEBUG -DHAVE_CONFIG_H
LDFLAGS:=

all: subscale

clean:
	rm -f *.o subscale

subscale: main.o format_sup.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.cpp common.hpp subtitle.hpp scale.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

format_sup.o: format_sup.cpp format_sup.hpp subtitle.hpp common.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
