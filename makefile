CC=gcc
CFLAGS= -g -Wall -std=c99
TARGETS= lexer parser
.PHONY= clean all

# Default commands
all: $(TARGETS)
	make $(TARGETS) 1>/dev/null

clean:
	rm $(TARGETS) *.o ./jack_samples_lexer/*_mine.txt 2>/dev/null

# Executable comp
lexer: lexer.o
	$(CC) $(CFLAGS) -o lexer lexer.o

parser: parser.o
	$(CC) $(CFLAGS) -o parser parser.o

# Object comp
lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c lexer.c -o lexer.o

parser.o: parser.c parser.h lexer.o
	$(CC) $(CFLAGS) -c parser.c -o parser.o
