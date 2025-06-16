# Cómo Crear Producciones Atribuidas para la Gramática

Este documento explica cómo crear producciones atribuidas basándose en el ejemplo de la función `test_grammar()` del archivo `test.h`.

## Conceptos Básicos

### ¿Qué es una Producción Atribuida?

Una producción atribuida (`AttrProd`) es una extensión de una producción regular que incluye una **acción semántica**. Esta acción semántica define cómo se calculan los atributos sintetizados del símbolo no terminal del lado izquierdo a partir de los atributos de los símbolos del lado derecho.

### Estructura de AttrProd

```cpp
class AttrProd : public Production {
public:
    using SemanticAction = std::function<ElementType(const std::vector<ElementType>& synthesized)>;
    
    AttrProd(std::shared_ptr<NonTerminal> left,
             const Sentence& right, 
             SemanticAction attr);
};
```

- **left**: Símbolo no terminal del lado izquierdo de la producción
- **right**: Sentencia (secuencia de símbolos) del lado derecho
- **attr**: Función lambda que define la acción semántica

## Tipos de Datos Importantes

### ElementType
```cpp
using ElementType = std::variant<
    std::string,
    std::shared_ptr<Node>,
    ASTNode*
>;
```

`ElementType` puede contener:
- Cadenas de texto (`std::string`)
- Nodos del AST (`std::shared_ptr<Node>`)
- Punteros a nodos AST (`ASTNode*`)

### Tipos de Nodos Disponibles

- **SymbolNode**: Representa un símbolo terminal
- **UnionNode**: Representa la unión de dos nodos
- **ConcatNode**: Representa la concatenación de dos nodos
- **ClosureNode**: Representa la clausura de Kleene de un nodo

## Ejemplo Práctico: Función test_grammar()

### Paso 1: Crear la Gramática y Definir Símbolos

```cpp
Grammar g;

// Definir símbolos no terminales
auto S = g.SetNonTerminal("S", true);  // true indica que es el símbolo inicial
auto A = g.SetNonTerminal("A");
auto B = g.SetNonTerminal("B");

// Definir símbolos terminales
auto a = g.SetTerminal("a");
auto b = g.SetTerminal("b");
```

### Paso 2: Crear Producciones Atribuidas

#### Producción 1: S → A B
```cpp
g.AddProduction(AttrProd(S, Sentence({A, B}), [](const std::vector<ElementType>& args) -> ElementType {
    return std::make_shared<UnionNode>(
        std::get<std::shared_ptr<Node>>(args[0]), 
        std::get<std::shared_ptr<Node>>(args[1])
    );
}));
```

**Explicación**:
- **Lado izquierdo**: `S`
- **Lado derecho**: `Sentence({A, B})` - secuencia de símbolos A y B
- **Acción semántica**: Lambda que toma los atributos sintetizados de A y B, y crea un `UnionNode`
- `args[0]` contiene el atributo sintetizado de A
- `args[1]` contiene el atributo sintetizado de B
- Se usa `std::get<std::shared_ptr<Node>>()` para extraer el nodo del `ElementType`

#### Producción 2: A → a
```cpp
g.AddProduction(AttrProd(A, Sentence(a), [](const std::vector<ElementType>& args) -> ElementType {
    return std::make_shared<SymbolNode>(std::get<std::string>(args[0]));
}));
```

**Explicación**:
- **Lado izquierdo**: `A`
- **Lado derecho**: `Sentence(a)` - símbolo terminal a
- **Acción semántica**: Crea un `SymbolNode` con el valor del terminal
- `args[0]` contiene el valor del terminal como `std::string`

#### Producción 3: B → b
```cpp
g.AddProduction(AttrProd(B, Sentence(b), [](const std::vector<ElementType>& args) -> ElementType {
    return std::make_shared<SymbolNode>(std::get<std::string>(args[0]));
}));
```

Similar a la producción anterior, pero para el símbolo B.

## Estructura General de una Acción Semántica

```cpp
[](const std::vector<ElementType>& args) -> ElementType {
    // 1. Extraer los valores de los argumentos usando std::get
    // 2. Procesar los valores según la semántica deseada
    // 3. Crear y retornar el resultado como ElementType
}
```

### Pasos para extraer valores:

1. **Para terminales (strings)**:
   ```cpp
   std::string terminal_value = std::get<std::string>(args[i]);
   ```

2. **Para no terminales (nodos)**:
   ```cpp
   std::shared_ptr<Node> node = std::get<std::shared_ptr<Node>>(args[i]);
   ```

## Buenas Prácticas

1. **Verificar tipos**: Siempre usar `std::get` con el tipo correcto para evitar excepciones
2. **Orden de argumentos**: Los argumentos en `args` corresponden al orden de los símbolos en el lado derecho
3. **Gestión de memoria**: Usar `std::shared_ptr` para gestión automática de memoria
4. **Semántica clara**: La acción semántica debe reflejar claramente la intención de la producción

## Ejemplo de Uso Completo

```cpp
int crear_gramatica_atribuida() {
    Grammar g;
    
    // Símbolos
    auto Expr = g.SetNonTerminal("Expr", true);
    auto Term = g.SetNonTerminal("Term");
    auto Factor = g.SetNonTerminal("Factor");
    auto plus = g.SetTerminal("+");
    auto mult = g.SetTerminal("*");
    auto num = g.SetTerminal("num");
    
    // Producciones atribuidas
    g.AddProduction(AttrProd(Expr, Sentence({Expr, plus, Term}), 
        [](const std::vector<ElementType>& args) -> ElementType {
            auto left = std::get<std::shared_ptr<Node>>(args[0]);
            auto right = std::get<std::shared_ptr<Node>>(args[2]);
            return std::make_shared<UnionNode>(left, right);
        }));
    
    g.AddProduction(AttrProd(Expr, Sentence({Term}), 
        [](const std::vector<ElementType>& args) -> ElementType {
            return args[0]; // Simplemente pasar el atributo
        }));
    
    // ... más producciones
    
    return 0;
}
```

Este enfoque permite construir ASTs durante el análisis sintáctico, facilitando las fases posteriores del compilador.
