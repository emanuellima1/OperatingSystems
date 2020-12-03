CXX=g++

INCLDIR	= include/

CXXFLAGS=-Wall -Wextra -std=gnu++17 -g -I$(INCLDIR)

SRC=$(wildcard src/*.cpp)
OBJ=$(SRC:.cpp=.o)
HEAD=$(wildcard include/*.hpp)

EXEC=ep3

$(EXEC): $(OBJ) $(HEAD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $(SRC)

clean:
	rm -rf $(OBJ) $(EXEC)
