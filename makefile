CC = g++
LEX = flex
YACC = bison
CFLAGS = -Wall -std=c++11

SRC_DIR = .
BUILDDIR = $(SRC_DIR)/build

PROGRAM = parser

OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/parser.tab.o $(BUILDDIR)/ast.o $(BUILDDIR)/main.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) -lfl

$(BUILDDIR)/parser.tab.c $(BUILDDIR)/parser.tab.h: $(SRC_DIR)/parser.y
	$(YACC) -d -o $(BUILDDIR)/parser.tab.c $(SRC_DIR)/parser.y

$(BUILDDIR)/lex.yy.c: $(SRC_DIR)/lexer.l $(BUILDDIR)/parser.tab.h
	$(LEX) -o $(BUILDDIR)/lex.yy.c $(SRC_DIR)/lexer.l

$(BUILDDIR)/%.o: $(BUILDDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.c
	$(CC) $(CFLAGS) -x c++ -c -o $@ $<

$(BUILDDIR)/parser.tab.o: $(BUILDDIR)/parser.tab.c
	$(CC) $(CFLAGS) -x c++ -c -o $@ $<

$(BUILDDIR)/ast.o: $(SRC_DIR)/ast.cpp $(SRC_DIR)/ast.hpp
	$(CC) $(CFLAGS) -c $(SRC_DIR)/ast.cpp -o $@

$(BUILDDIR)/main.o: $(SRC_DIR)/main.cpp $(BUILDDIR)/parser.tab.h $(SRC_DIR)/ast.hpp
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.cpp -o $@

clean:
	rm -f $(BUILDDIR)/* $(PROGRAM)

.PHONY: all clean