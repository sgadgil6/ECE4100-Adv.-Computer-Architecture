CXXFLAGS := -g -Wall -lm

ifdef C
CXX:=cc
CXXFLAGS += -DCCOMPILER
else
CXX := g++
CXXFLAGS += -std=c++0x
endif

all: cachesim

cachesim: cachesim.o cachesim_driver.o
	$(CXX) -o $@ $^ $(LDFLAGS)

cachesim.o: cachesim.cpp cachesim.hpp
	$(CXX) -c $(CXXFLAGS) $<

cachesim_driver.o: cachesim_driver.cpp
	$(CXX) -c $(CXXFLAGS) $<

clean:
	rm -f cachesim *.o
