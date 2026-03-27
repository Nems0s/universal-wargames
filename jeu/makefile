CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g

SRC = $(wildcard *.cc)
OBJ = $(SRC:.cc=.o)
EXEC = main

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXEC)