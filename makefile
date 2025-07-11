CC = clang++
LEX = flex
YACC = bison
CFLAGS = -Wall -std=c++17 -fexceptions

LLVM_CONFIG=llvm-config-15

CXXFLAGS=$(shell $(LLVM_CONFIG) --cxxflags | sed 's/-std=c++[0-9][0-9]/-std=c++17/') -fexceptions
LDFLAGS=`$(LLVM_CONFIG) --ldflags --libs all`

SRC_DIR = .
BUILDDIR = $(SRC_DIR)/hulk

PROGRAM = hulk

# Include all necessary source files for grammar, automata, parser, and AST
GRAMMAR_SRC = $(SRC_DIR)/Grammar/grammar.cpp $(SRC_DIR)/Grammar/production.cpp $(SRC_DIR)/Grammar/symbol.cpp $(SRC_DIR)/Grammar/sentence.cpp
AUTOMATA_SRC = $(SRC_DIR)/Automata/dfa.cpp $(SRC_DIR)/Automata/nfa.cpp $(SRC_DIR)/Automata/state.cpp $(SRC_DIR)/Automata/utils/ContainerSet.cpp $(SRC_DIR)/Automata/utils/aut_manipulation.cpp $(SRC_DIR)/Automata/operations/operations.cpp
PARSER_SRC = $(SRC_DIR)/Parser/Item.cpp $(SRC_DIR)/Parser/LALR1Parser.cpp
LOGGER_SRC = $(SRC_DIR)/Logger/Logger.cpp
EXECUTE_SRC = $(SRC_DIR)/execute.cpp

# Add object files for the grammar, automata, and parser
GRAMMAR_OBJS = $(BUILDDIR)/grammar.o $(BUILDDIR)/production.o $(BUILDDIR)/symbol.o $(BUILDDIR)/sentence.o
AUTOMATA_OBJS = $(BUILDDIR)/dfa.o $(BUILDDIR)/nfa.o $(BUILDDIR)/state.o $(BUILDDIR)/ContainerSet.o $(BUILDDIR)/aut_manipulation.o $(BUILDDIR)/operations.o
PARSER_OBJS = $(BUILDDIR)/Item.o $(BUILDDIR)/LALR1Parser.o
LOGGER_OBJS = $(BUILDDIR)/Logger.o
EXECUTE_OBJS = $(BUILDDIR)/execute.o

# Add the additional objects to the main objects list
MAIN_OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/parser.tab.o $(BUILDDIR)/ast.o $(BUILDDIR)/main.o $(BUILDDIR)/context.o $(BUILDDIR)/visitor.o $(BUILDDIR)/type_collector_visitor.o $(BUILDDIR)/symbol_collector_visitor.o $(BUILDDIR)/codegen.o $(BUILDDIR)/jit.o $(BUILDDIR)/Logger.o
OBJS = $(MAIN_OBJS) $(GRAMMAR_OBJS) $(AUTOMATA_OBJS) $(PARSER_OBJS) $(EXECUTE_OBJS)

# Define los archivos necesarios para el create_artifacts
# Incluimos Ast/ast.cpp porque el código de gramática depende de clases AST
ARTIFACTS_SRC = $(SRC_DIR)/create_artifacts.cpp $(SRC_DIR)/execute.cpp $(SRC_DIR)/hulkGrammar.hpp \
                $(SRC_DIR)/Grammar/grammar.cpp $(SRC_DIR)/Grammar/production.cpp \
                $(SRC_DIR)/Grammar/symbol.cpp $(SRC_DIR)/Grammar/sentence.cpp \
                $(SRC_DIR)/Automata/dfa.cpp $(SRC_DIR)/Automata/nfa.cpp \
                $(SRC_DIR)/Automata/utils/ContainerSet.cpp $(SRC_DIR)/Automata/utils/aut_manipulation.cpp \
                $(SRC_DIR)/Automata/operations/operations.cpp $(SRC_DIR)/Automata/state.cpp \
                $(SRC_DIR)/Parser/Item.cpp $(SRC_DIR)/Parser/LALR1Parser.cpp \
                $(SRC_DIR)/Ast/ast.cpp

compile: $(BUILDDIR)/libstandard.so  $(BUILDDIR)/$(PROGRAM)

#create_artifacts: | $(BUILDDIR)
#	$(CC) $(CFLAGS) -o $(SRC_DIR)/create_artifacts $(SRC_DIR)/create_artifacts.cpp $(SRC_DIR)/execute.cpp \
#		$(SRC_DIR)/Grammar/grammar.cpp $(SRC_DIR)/Grammar/production.cpp \
#		$(SRC_DIR)/Grammar/symbol.cpp $(SRC_DIR)/Grammar/sentence.cpp \
#		$(SRC_DIR)/Automata/dfa.cpp $(SRC_DIR)/Automata/nfa.cpp \
#		$(SRC_DIR)/Automata/utils/ContainerSet.cpp $(SRC_DIR)/Automata/utils/aut_manipulation.cpp \
#		$(SRC_DIR)/Automata/operations/operations.cpp $(SRC_DIR)/Automata/state.cpp \
#		$(SRC_DIR)/Parser/Item.cpp $(SRC_DIR)/Parser/LALR1Parser.cpp \
#		$(SRC_DIR)/Logger/Logger.cpp \
#		$(SRC_DIR)/Ast/ast.cpp
#	$(SRC_DIR)/create_artifacts

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/libstandard.so: $(SRC_DIR)/codegen/standard.cpp | $(BUILDDIR)
	$(CC) -shared -fPIC -o $(BUILDDIR)/libstandard.so $(SRC_DIR)/codegen/standard.cpp

$(BUILDDIR)/$(PROGRAM): $(OBJS) $(BUILDDIR)/libstandard.so
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILDDIR)/$(PROGRAM) -lfl -L$(BUILDDIR) -lstandard $(LDFLAGS)

#$(BUILDDIR)/parser.tab.c $(BUILDDIR)/parser.tab.h: $(SRC_DIR)/parser.y | $(BUILDDIR)
#	$(YACC) -d -o $(BUILDDIR)/parser.tab.c $(SRC_DIR)/parser.y

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

$(BUILDDIR)/main.o: $(SRC_DIR)/main.cpp $(BUILDDIR)/parser.tab.h $(SRC_DIR)/Ast/ast.hpp $(SRC_DIR)/semantic/visitor.h $(SRC_DIR)/semantic/visitor.cpp
	$(CC) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp $(LDFLAGS) -o $@

$(BUILDDIR)/type_collector_visitor.o: $(SRC_DIR)/semantic/type_collector_visitor.cpp $(SRC_DIR)/semantic/type_collector_visitor.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/semantic/type_collector_visitor.cpp -o $@

$(BUILDDIR)/symbol_collector_visitor.o: $(SRC_DIR)/semantic/symbol_collector_visitor.cpp $(SRC_DIR)/semantic/symbol_collector_visitor.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/semantic/symbol_collector_visitor.cpp -o $@
	
# Rules for Grammar objects
$(BUILDDIR)/grammar.o: $(SRC_DIR)/Grammar/grammar.cpp $(SRC_DIR)/Grammar/grammar.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Grammar/grammar.cpp -o $@

$(BUILDDIR)/production.o: $(SRC_DIR)/Grammar/production.cpp $(SRC_DIR)/Grammar/production.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Grammar/production.cpp -o $@

$(BUILDDIR)/symbol.o: $(SRC_DIR)/Grammar/symbol.cpp $(SRC_DIR)/Grammar/symbol.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Grammar/symbol.cpp -o $@

$(BUILDDIR)/sentence.o: $(SRC_DIR)/Grammar/sentence.cpp $(SRC_DIR)/Grammar/sentence.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Grammar/sentence.cpp -o $@

# Rule for Logger object
$(BUILDDIR)/Logger.o: $(SRC_DIR)/Logger/Logger.cpp $(SRC_DIR)/Logger/Logger.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Logger/Logger.cpp -o $@

# Rules for Automata objects
$(BUILDDIR)/dfa.o: $(SRC_DIR)/Automata/dfa.cpp $(SRC_DIR)/Automata/dfa.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Automata/dfa.cpp -o $@

$(BUILDDIR)/nfa.o: $(SRC_DIR)/Automata/nfa.cpp $(SRC_DIR)/Automata/nfa.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Automata/nfa.cpp -o $@

$(BUILDDIR)/state.o: $(SRC_DIR)/Automata/state.cpp $(SRC_DIR)/Automata/state.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Automata/state.cpp -o $@

$(BUILDDIR)/ContainerSet.o: $(SRC_DIR)/Automata/utils/ContainerSet.cpp $(SRC_DIR)/Automata/utils/ContainerSet.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Automata/utils/ContainerSet.cpp -o $@

$(BUILDDIR)/aut_manipulation.o: $(SRC_DIR)/Automata/utils/aut_manipulation.cpp $(SRC_DIR)/Automata/utils/aut_manipulation.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Automata/utils/aut_manipulation.cpp -o $@

$(BUILDDIR)/operations.o: $(SRC_DIR)/Automata/operations/operations.cpp $(SRC_DIR)/Automata/operations/operations.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Automata/operations/operations.cpp -o $@

# Rules for Parser objects
$(BUILDDIR)/Item.o: $(SRC_DIR)/Parser/Item.cpp $(SRC_DIR)/Parser/Item.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Parser/Item.cpp -o $@

$(BUILDDIR)/LALR1Parser.o: $(SRC_DIR)/Parser/LALR1Parser.cpp $(SRC_DIR)/Parser/LALR1Parser.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Parser/LALR1Parser.cpp -o $@

# Rule for execute object
$(BUILDDIR)/execute.o: $(SRC_DIR)/execute.cpp $(SRC_DIR)/execute.h | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/execute.cpp -o $@

clean:
	rm -rf $(BUILDDIR)/*.o $(BUILDDIR)/*.c $(BUILDDIR)/*.h $(BUILDDIR)/$(PROGRAM) $(BUILDDIR)/libstandard.so $(SRC_DIR)/create_artifacts
	rm -f $(BUILDDIR)/lexer.l $(BUILDDIR)/parser.p $(BUILDDIR)/hulk_parser.p

execute: compile
	cd $(SRC_DIR) && LD_LIBRARY_PATH=./hulk ./hulk/$(PROGRAM) ./script.hulk

.PHONY: compile execute clean