# Parser LR(1) para Hulk

Este proyecto implementa un parser LR(1) completo para el lenguaje Hulk, utilizando técnicas de construcción de compiladores desde cero.

## Componentes Principales

1. **LR1Item.h/cpp**: Implementa los ítems LR(1), que son extensiones de los ítems LR(0) con tokens de anticipación (lookaheads).

2. **LR1Parser.h/cpp**: Implementa el algoritmo de parsing LR(1) completo, construyendo las tablas ACTION y GOTO específicas para LR(1).

3. **test_lr1_parser.cpp**: Un programa de prueba simple que demuestra el funcionamiento del parser con una gramática de expresiones booleanas.

4. **hulk_lr1_parser.cpp**: Integra el parser LR(1) con la gramática completa de Hulk.

## Diferencias con SLR(1)

El parser LR(1) es más potente que el SLR(1) porque:

- Utiliza lookaheads específicos para cada ítem, no solo los conjuntos FOLLOW globales.
- Puede resolver más conflictos y manejar más gramáticas.
- Genera estados más precisos en el autómata.

## Compilación y Ejecución

Se proporciona un script `run_lr1_parser.sh` para facilitar la compilación y ejecución.

```bash
./run_lr1_parser.sh
```

También se puede compilar manualmente:

```bash
# Para compilar ambos ejemplos
make -f Makefile.lr1

# Para compilar solo el test simple
make -f Makefile.lr1 test

# Para compilar solo el parser Hulk
make -f Makefile.lr1 hulk
```

Ejecución manual:

```bash
# Para el test simple de expresiones booleanas
./test_lr1_parser

# Para analizar un script Hulk
./hulk_lr1_parser script.hulk
```

## Algoritmo LR(1)

El algoritmo LR(1) implementado sigue estos pasos:

1. **Cálculo de Conjuntos FIRST**: Para cada símbolo y secuencia de símbolos.

2. **Construcción de Ítems LR(1)**: Ítems con lookaheads específicos.

3. **Clausura LR(1)**: Algoritmo para completar un conjunto de ítems con todas sus derivaciones.

4. **Construcción del Autómata**: Creación de estados y transiciones.

5. **Construcción de Tablas ACTION y GOTO**: Determina qué acción tomar para cada par (estado, token).

6. **Resolución de Conflictos**: Estrategias para resolver conflictos Shift/Reduce y Reduce/Reduce.

7. **Análisis Sintáctico**: Uso de un autómata de pila para analizar la entrada.

## Notas de Implementación

- La creación de los ítems LR(1) mantiene los ítems LR(0) como base para reutilizar código.
- Se implementa el cálculo eficiente de conjuntos FIRST con memoización.
- Se incluye manejo detallado de errores con reportes claros.
- El código está diseñado para ser extensible a otros tipos de parsers.

## Integración con la Gramática Hulk

El parser LR(1) es compatible con la gramática SLR(1) modificada de Hulk, pero tiene mejor capacidad para manejar construcciones complejas. La integración permite:

1. Análisis de expresiones anidadas.
2. Manejo correcto de ambigüedades mediante la especificación de precedencias.
3. Mejor diagnóstico de errores sintácticos.

## Ejemplo de Uso en Código

```cpp
// Crear la gramática
Grammar grammar;
DefineHulkGrammar(grammar);

// Crear el parser LR(1)
LR1Parser parser(grammar);

// Analizar una secuencia de tokens
auto result = parser.Parse(tokens);
```
