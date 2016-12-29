CFLAGS=-Wall -Wextra  -O3 -Werror
LFLAGS=-lpng -ljpeg -ltiff -fopenmp -lm


# Recursively get all *.cpp and *.c in this directory and any sub-directories
SRC1 := $(shell find . -name "*.cpp") 
SRC2 := $(shell find . -name "*.c") 

INCLUDE = -I.

#Replace suffix .cpp and .c by .o
OBJ := $(addsuffix .o,$(basename $(SRC1))) $(addsuffix .o,$(basename $(SRC2)))

#Binary file
BIN  = main 
DEST = inverse_compositional_algorithm 

OBJBIN = ./main.o
OBJ1 := $(filter-out $(OBJBIN),$(OBJ))

#All is the target (you would run make all from the command line). 'all' is dependent
all: $(BIN) 

#Generate executables
main: $(OBJ1) main.o
	g++ -std=c++11 $(OBJ1) main.o -o inverse_compositional_algorithm $(CFLAGS) $(LFLAGS) -lstdc++


#each object file is dependent on its source file, and whenever make needs to create
#an object file, to follow this rule:
%.o: %.c
	gcc -std=c99  -c $< -o $@ $(INCLUDE) $(CFLAGS) $(LFLAGS)  -Wno-unused -pedantic -DNDEBUG -D_GNU_SOURCE

%.o: %.cpp
	g++ -std=c++11 -c $< -o $@ $(INCLUDE) $(CFLAGS) $(LFLAGS) 

clean: 
	rm -f $(OBJ) $(DEST)
