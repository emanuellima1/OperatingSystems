CC=g++

INCLDIR	= include/

CFLAGS=-Wall -Wextra -std=gnu++17 -g -I$(INCLDIR)

SRC=$(wildcard src/*.cpp)
OBJ=$(SRC:.cpp=.o)
HEAD=$(wildcard include/*.hpp)

EXEC=ep3

$(EXEC): $(OBJ) $(HEAD)
	$(CC) $(CFLAGS) -o $@ $(SRC)


clean:
	rm -rf $(OBJ) $(EXEC)
