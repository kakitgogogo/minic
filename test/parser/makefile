CC		= g++
CXXFLAG	= -std=c++11
INCL	= -I ../../src
TEST	= t

.PHONY: all clean

all: $(TEST)

clean:
	rm -f $(TEST)

t: t.cpp ../../src/lexer.cpp ../../src/token.cpp ../../src/parser.cpp
	$(CC) $(CXXFLAG) $(INCL) -o $@ $^
