CXX=g++

INCLDIR	= include/

CXXFLAGS=-Wall -Wextra -pedantic -std=gnu++17 -I$(INCLDIR)

SRC=$(wildcard src/*.cpp)
BENCH=$(wildcard benchmark/*.cpp)
BENCH_DEPS=$(filter-out src/ep3.cpp, $(wildcard src/*.cpp))
OBJ=$(SRC:.cpp=.o)
OBJ2=$(BENCH:.cpp=.o)
OBJ_BENCH_DEPS=$(BENCH_DEPS:.cpp=.o)
HEAD=$(wildcard include/*.hpp)

EXEC=ep3

$(EXEC): $(OBJ) $(HEAD)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

bench: $(OBJ2) $(OBJ_BENCH_DEPS) $(HEAD)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ2) $(OBJ_BENCH_DEPS)

clean:
	rm -rf $(OBJ) $(EXEC) bench
