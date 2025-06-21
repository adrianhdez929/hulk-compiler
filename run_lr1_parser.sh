#!/bin/bash

# Script para compilar y ejecutar el parser LR1

# Colores para mensajes
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Compilando el parser LR1 ===${NC}"
make -f Makefile.lr1 clean && make -f Makefile.lr1

# Verificar si la compilación fue exitosa
if [ $? -ne 0 ]; then
    echo -e "${RED}Error en la compilación.${NC}"
    exit 1
fi

echo -e "${GREEN}Compilación exitosa.${NC}"

# Ejecutar el test simple
echo -e "\n${BLUE}=== Ejecutando prueba simple ===${NC}"
./test_lr1_parser

# Verificar si el test fue exitoso
if [ $? -ne 0 ]; then
    echo -e "${RED}Error en el test simple.${NC}"
    exit 1
fi

# Preguntar si se desea ejecutar el parser con un script Hulk
echo -e "\n${BLUE}=== ¿Desea ejecutar el parser con un script Hulk? (s/n) ===${NC}"
read -r response
if [[ "$response" =~ ^([sS][iI]|[sS])$ ]]; then
    echo -e "\n${BLUE}=== Ingrese el nombre del archivo Hulk ===${NC}"
    read -r hulk_file
    if [ -f "$hulk_file" ]; then
        echo -e "${GREEN}Ejecutando parser con $hulk_file...${NC}"
        ./hulk_lr1_parser "$hulk_file"
    else
        echo -e "${RED}El archivo $hulk_file no existe.${NC}"
        exit 1
    fi
else
    echo -e "${BLUE}No se ejecutará el parser con un script Hulk.${NC}"
fi

echo -e "\n${GREEN}Proceso completado.${NC}"
exit 0
