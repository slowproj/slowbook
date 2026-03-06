
SLOWBOOK_DIR=../..

CXX=g++
CXXFLAGS=-std=c++17 -I$(SLOWBOOK_DIR)/includes
LIBS=
BINS = $(shell echo *.cpp | sed "s/\.cpp//g")
INCS = $(shell echo $(SLOWBOOK_DIR)/includes/slowbook/*.hpp)


# To use Redis  (setup on Ubuntu 24.04: sudo apt install libhiredis-dev)
CXXFLAGS+=$(shell pkg-config --cflags hiredis)
LIBS+=$(shell pkg-config --libs hiredis)



all: $(BINS)


%: %.cpp $(INCS)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)


clean:
	-rm -f $(BINS)
	-rm -f *.o
