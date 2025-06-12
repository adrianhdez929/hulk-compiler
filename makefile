CC = clang++
LEX = flex
YACC = bison
CFLAGS = -Wall -std=c++11 -fexceptions

LLVM_CONFIG=llvm-config-15

CXXFLAGS=$(shell $(LLVM_CONFIG) --cxxflags) -fexceptions
LDFLAGS=`$(LLVM_CONFIG) --ldflags --libs all`

SRC_DIR = .
BUILDDIR = $(SRC_DIR)/hulk

PROGRAM = hulk

OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/parser.tab.o $(BUILDDIR)/ast.o $(BUILDDIR)/main.o $(BUILDDIR)/context.o $(BUILDDIR)/visitor.o $(BUILDDIR)/codegen.o $(BUILDDIR)/jit.o

compile: $(BUILDDIR)/libstandard.so $(BUILDDIR)/$(PROGRAM)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/libstandard.so: $(SRC_DIR)/codegen/standard.cpp | $(BUILDDIR)
	$(CC) -shared -fPIC -o $(BUILDDIR)/libstandard.so $(SRC_DIR)/codegen/standard.cpp

$(BUILDDIR)/$(PROGRAM): $(OBJS) $(BUILDDIR)/libstandard.so
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILDDIR)/$(PROGRAM) -lfl -L$(BUILDDIR) -lstandard $(LDFLAGS)

$(BUILDDIR)/parser.tab.c $(BUILDDIR)/parser.tab.h: $(SRC_DIR)/parser.y | $(BUILDDIR)
	$(YACC) -d -o $(BUILDDIR)/parser.tab.c $(SRC_DIR)/parser.y

$(BUILDDIR)/lex.yy.c: $(SRC_DIR)/lexer.l $(BUILDDIR)/parser.tab.h
	$(LEX) -o $(BUILDDIR)/lex.yy.c $(SRC_DIR)/lexer.l

$(BUILDDIR)/%.o: $(BUILDDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.c
	$(CC) $(CFLAGS) -x c++ -c -o $@ $<

$(BUILDDIR)/parser.tab.o: $(BUILDDIR)/parser.tab.c
	$(CC) $(CFLAGS) -x c++ -c -o $@ $<

$(BUILDDIR)/ast.o: $(SRC_DIR)/Ast/ast.cpp $(SRC_DIR)/Ast/ast.hpp | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Ast/ast.cpp -o $@

$(BUILDDIR)/context.o: $(SRC_DIR)/semantic/context.cpp $(SRC_DIR)/semantic/context.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/semantic/context.cpp -o $@

$(BUILDDIR)/visitor.o: $(SRC_DIR)/semantic/visitor.cpp $(SRC_DIR)/semantic/visitor.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/semantic/visitor.cpp -o $@

$(BUILDDIR)/codegen.o: $(SRC_DIR)/codegen/visitor.cpp $(SRC_DIR)/codegen/visitor.h | $(BUILDDIR)
	$(CC) $(CXXFLAGS) -c $(SRC_DIR)/codegen/visitor.cpp -o $@

$(BUILDDIR)/jit.o: $(SRC_DIR)/codegen/jit.cpp $(SRC_DIR)/codegen/jit.h | $(BUILDDIR)
	$(CC) $(CXXFLAGS) -c $(SRC_DIR)/codegen/jit.cpp -o $@

$(BUILDDIR)/main.o: $(SRC_DIR)/main.cpp $(BUILDDIR)/parser.tab.h $(SRC_DIR)/Ast/ast.hpp $(SRC_DIR)/semantic/visitor.h $(SRC_DIR)/semantic/visitor.cpp | $(BUILDDIR)
	$(CC) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $@

clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.c $(BUILDDIR)/*.h $(BUILDDIR)/$(PROGRAM) $(BUILDDIR)/libstandard.so

execute:
	@if [ ! -d "$(BUILDDIR)" ] || [ -z "$$(ls -A $(BUILDDIR) 2>/dev/null)" ]; then \
		echo "Build directory is empty or missing, compiling..."; \
		$(MAKE) compile; \
	fi
	cd $(BUILDDIR) && LD_LIBRARY_PATH=. ./$(PROGRAM) ../script.txt

.PHONY: compile execute clean