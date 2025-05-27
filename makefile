CC = clang++
LEX = flex
YACC = bison
CFLAGS = -Wall -std=c++11 -fexceptions

LLVM_CONFIG=llvm-config-15

CXXFLAGS=$(shell $(LLVM_CONFIG) --cxxflags) -fexceptions
LDFLAGS=`$(LLVM_CONFIG) --ldflags --libs all`

SRC_DIR = .
BUILDDIR = $(SRC_DIR)/build

PROGRAM = parser

OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/parser.tab.o $(BUILDDIR)/ast.o $(BUILDDIR)/main.o $(BUILDDIR)/context.o $(BUILDDIR)/visitor.o $(BUILDDIR)/codegen.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) -lfl $(LDFLAGS)

$(BUILDDIR)/parser.tab.c $(BUILDDIR)/parser.h: $(SRC_DIR)/parser.y
	$(YACC) -d -o $(BUILDDIR)/parser.tab.c $(SRC_DIR)/parser.y

$(BUILDDIR)/lex.yy.c: $(SRC_DIR)/lexer.l $(BUILDDIR)/parser.h
	$(LEX) -o $(BUILDDIR)/lex.yy.c $(SRC_DIR)/lexer.l

$(BUILDDIR)/%.o: $(BUILDDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.c
	$(CC) $(CFLAGS) -x c++ -c -o $@ $<

$(BUILDDIR)/parser.tab.o: $(BUILDDIR)/parser.tab.c
	$(CC) $(CFLAGS) -x c++ -c -o $@ $<

$(BUILDDIR)/ast.o: $(SRC_DIR)/Ast/ast.cpp $(SRC_DIR)/Ast/ast.hpp
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Ast/ast.cpp -o $@

$(BUILDDIR)/context.o: $(SRC_DIR)/semantic/context.cpp $(SRC_DIR)/semantic/context.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/semantic/context.cpp -o $@

$(BUILDDIR)/visitor.o: $(SRC_DIR)/semantic/visitor.cpp $(SRC_DIR)/semantic/visitor.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/semantic/visitor.cpp -o $@

$(BUILDDIR)/codegen.o: $(SRC_DIR)/codegen/visitor.cpp $(SRC_DIR)/codegen/visitor.h
	$(CC) $(CXXFLAGS) -c $(SRC_DIR)/codegen/visitor.cpp $(LDFLAGS) -o $@

$(BUILDDIR)/main.o: $(SRC_DIR)/main.cpp $(BUILDDIR)/parser.tab.h $(SRC_DIR)/Ast/ast.hpp $(SRC_DIR)/semantic/visitor.h $(SRC_DIR)/semantic/visitor.cpp
	$(CC) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp $(LDFLAGS) -o $@

clean:
	rm -f $(BUILDDIR)/* $(PROGRAM)

.PHONY: all clean