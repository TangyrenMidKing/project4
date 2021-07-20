# to run in terminal $ make
CC = gcc

# List all your .c (or .cpp or .cc) files here (source files, excluding header files)
SRC_CODE = driver.c lex.o parser.o codegen.c

#################################

# default rule

all: $(SRC_CODE)
	$(CC) $(SRC_CODE) -lm



