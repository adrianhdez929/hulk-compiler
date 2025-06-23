#!/bin/bash

# Script para compilar el archivo print_parser_tables.cpp

# Definir el compilador y las rutas
CC=g++
CFLAGS="-std=c++17 -g"
SRC_DIR="."
OUTPUT="print_parser_tables"

echo "Compilando print_parser_tables.cpp..."

# Compilar con las mismas dependencias que el resto del proyecto
$CC $CFLAGS -o $OUTPUT $SRC_DIR/print_parser_tables.cpp \
    $SRC_DIR/Grammar/grammar.cpp \
    $SRC_DIR/Grammar/production.cpp \
    $SRC_DIR/Grammar/symbol.cpp \
    $SRC_DIR/Grammar/sentence.cpp \
    $SRC_DIR/Automata/dfa.cpp \
    $SRC_DIR/Automata/nfa.cpp \
    $SRC_DIR/Automata/utils/ContainerSet.cpp \
    $SRC_DIR/Automata/utils/aut_manipulation.cpp \
    $SRC_DIR/Automata/operations/operations.cpp \
    $SRC_DIR/Automata/state.cpp \
    $SRC_DIR/Parser/Item.cpp \
    $SRC_DIR/Parser/LR1Parser.cpp \
    $SRC_DIR/Parser/SLR1Parser.cpp \
    $SRC_DIR/Ast/ast.cpp

# Verificar si la compilación fue exitosa
if [ $? -eq 0 ]; then
    echo "Compilación exitosa!"
    echo "Para ejecutar el programa use: ./print_parser_tables"
    chmod +x $OUTPUT
else
    echo "Error en la compilación"
fi
