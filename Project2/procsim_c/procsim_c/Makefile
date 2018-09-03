CXXFLAGS := -g -Wall -lm 
CXX=gcc
SRC=procsim.c procsim_driver.c
PROCSIM=./procsim
R=12
J=1
K=2
L=3
F=4
P=32

build:
	$(CXX) $(CXXFLAGS) $(SRC) -o procsim

run:
	$(PROCSIM) -r$R -f$F -j$J -k$K -l$L -p$P < traces/mcf.100k.trace 

clean:
	rm -f procsim *.o
