PRG = MemStax.exe
PRG_D = MemStax_D.exe
PRG_TEST = MemStax_UnitTests.exe

GCC = g++

GCCFLAGS_D = -std=c++17 -Wall -Wextra -g -O0 -pedantic -DDEBUG -g
GCCFLAGS = -std=c++17 -Wall -Wextra

SRC = ./src/memstaxtest.cpp ./src/memstax.h 
SRC_TEST = ./src/memstaxtest.cpp ./src/memstax.h 
LIB =

run: gcc
	@./$(PRG)

gcc:
	@$(GCC) -o $(PRG) $(SRC) $(LIB) $(GCCFLAGS) 

# Runs without compiling
nocomp:
	@./$(PRG)

# Run debug program 
d: gcc_d
	./$(PRG_D)

# Configure and compile for debug
gcc_d:
	$(GCC) -o $(PRG_D) $(SRC) $(LIB) $(GCCFLAGS_D)

# Run all unit tests
unittest: gcc_ut
	@./$(PRG_TEST)

# Configure and compile for unit testing
gcc_ut:
	$(GCC) -o $(PRG_TEST) $(SRC_TEST) $(LIB) $(GCCFLAGS_D)

clean:
	rm -f $(PRG) $(PRG_D) $(PRG_TEST)
