CC=g++

INCLDIR	= include/

CFLAGS=-Wall -Wextra -std=gnu++17 -g -I$(INCLDIR)

SRC=$(wildcard src/*.cpp)
OBJ=$(SRC1:.cpp=.o)

EXEC=ep3

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -rf $(OBJ) $(EXEC)