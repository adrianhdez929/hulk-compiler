# Serialización del Lexer

## Descripción

Este sistema permite guardar (serializar) y cargar (deserializar) un Lexer completo en un archivo binario. Esto es útil para:

- **Acelerar el inicio del compilador**: Una vez que tienes tu lexer configurado, puedes guardarlo y cargarlo rápidamente en futuras ejecuciones
- **Distribuir lexers precompilados**: Puedes crear lexers y distribuirlos sin necesidad de recompilar
- **Cachear configuraciones complejas**: Para lexers con muchas reglas complejas, la serialización evita reconstruir todo el autómata

## Cómo Funciona

El sistema serializa el autómata determinista final del lexer, que contiene toda la información necesaria para tokenizar:

- **Estados**: ID, si es final, tag, metadatos
- **Transiciones**: Conexiones entre estados con símbolos
- **Transiciones epsilon**: Conexiones directas entre estados
- **Estructura del autómata**: Estado inicial y relaciones

## Uso Básico

### 1. Serializar un Lexer

```cpp
// Crear tu lexer normalmente
std::vector<std::pair<std::string, std::string>> token_table = {
    {"NUMBER", "[0-9]+"},
    {"IDENTIFIER", "[a-zA-Z][a-zA-Z0-9]*"},
    {"PLUS", "\\+"}
};

Grammar grammar;
SLR1Parser parser(grammar, false);
Lexer mi_lexer(token_table, grammar, parser);

// OPCIÓN 1: Guardarlo automáticamente en la carpeta hulk/
bool exito = mi_lexer.serialize_lexer("mi_lexer.bin");
// Esto crea el archivo: hulk/mi_lexer.bin

// OPCIÓN 2: Guardarlo en una carpeta específica
bool exito2 = mi_lexer.serialize_lexer("mi_lexer.bin", "build");
// Esto crea el archivo: build/mi_lexer.bin

// OPCIÓN 3: Guardarlo en la carpeta actual
bool exito3 = mi_lexer.serialize_lexer("mi_lexer.bin", "");
// Esto crea el archivo: mi_lexer.bin

if (exito) {
    std::cout << "Lexer guardado exitosamente!" << std::endl;
}
```

### 2. Deserializar un Lexer

```cpp
// OPCIÓN 1: Cargar desde la carpeta hulk/ (automático)
Lexer* lexer_hulk = Lexer::deserialize_lexer("mi_lexer.bin");

// OPCIÓN 2: Cargar desde una carpeta específica
Lexer* lexer_build = Lexer::deserialize_lexer("mi_lexer.bin", "build");

// OPCIÓN 3: Cargar desde la carpeta actual
Lexer* lexer_local = Lexer::deserialize_lexer("mi_lexer.bin", "");

if (lexer_hulk) {
    // Usar el lexer normalmente
    auto tokens = lexer_hulk->tokenize("variable = 123");
    
    // No olvides liberar la memoria
    delete lexer_hulk;
} else {
    std::cerr << "Error al cargar el lexer" << std::endl;
}
```

### 3. Patrón de Uso Recomendado para Proyectos

```cpp
Lexer* obtener_lexer_proyecto() {
    // Para proyectos, recomendamos usar la carpeta hulk/
    const std::string archivo_lexer = "lexer_cache.bin";
    
    // Intentar cargar desde hulk/ primero
    Lexer* lexer = Lexer::deserialize_lexer(archivo_lexer);
    
    if (!lexer) {
        // Si no existe, crear nuevo lexer
        std::cout << "Creando nuevo lexer..." << std::endl;
        
        // Tu código de creación del lexer aquí
        std::vector<std::pair<std::string, std::string>> tokens = {...};
        Grammar grammar;
        SLR1Parser parser(grammar, false);
        
        lexer = new Lexer(tokens, grammar, parser);
        
        // Guardarlo en hulk/ para la próxima vez
        lexer->serialize_lexer(archivo_lexer);
        std::cout << "Lexer guardado en hulk/lexer_cache.bin" << std::endl;
    } else {
        std::cout << "Lexer cargado desde hulk/lexer_cache.bin" << std::endl;
    }
    
    return lexer;
}
```

## Formato del Archivo

El archivo binario contiene:

1. **Cabecera**:
   - Número de estados (size_t)
   - ID del estado inicial (int)

2. **Para cada estado**:
   - ID del estado (int)
   - Si es final (bool)
   - Valor n (int)
   - Tag (longitud + string)
   - Transiciones normales (cantidad + [símbolo + destinos])
   - Transiciones epsilon (cantidad + IDs)

## Gestión de Carpetas

El sistema de serialización ofrece flexibilidad para organizar tus archivos:

### Carpeta Hulk (Por Defecto)
```cpp
// Estos métodos usan automáticamente la carpeta hulk/
lexer.serialize_lexer("mi_lexer.bin");           // Guarda en: hulk/mi_lexer.bin
Lexer* lexer = Lexer::deserialize_lexer("mi_lexer.bin");  // Carga desde: hulk/mi_lexer.bin
```

**Ventajas de usar hulk/:**
- Mantiene organizados los archivos del compilador
- Evita contaminar el directorio raíz del proyecto
- Es la estructura estándar del proyecto

### Carpetas Personalizadas
```cpp
// Especificar cualquier carpeta
lexer.serialize_lexer("cache.bin", "build");     // Guarda en: build/cache.bin
lexer.serialize_lexer("temp.bin", "tmp");        // Guarda en: tmp/temp.bin
lexer.serialize_lexer("backup.bin", "../backup"); // Guarda en: ../backup/backup.bin

// Para cargar desde carpetas personalizadas
Lexer* lexer1 = Lexer::deserialize_lexer("cache.bin", "build");
Lexer* lexer2 = Lexer::deserialize_lexer("temp.bin", "tmp");
```

### Carpeta Actual
```cpp
// Usar string vacío para la carpeta actual
lexer.serialize_lexer("local.bin", "");          // Guarda en: local.bin
Lexer* lexer = Lexer::deserialize_lexer("local.bin", "");  // Carga desde: local.bin
```

### Creación Automática de Carpetas
El sistema automáticamente:
- Verifica si la carpeta existe
- Crea la carpeta si no existe (usando `mkdir -p`)
- Reporta errores si no puede crear la carpeta

```cpp
// Si "mi_carpeta" no existe, se creará automáticamente
bool exito = lexer.serialize_lexer("archivo.bin", "mi_carpeta");
```
## Consideraciones

### Ventajas
- **Velocidad**: Cargar un lexer serializado es mucho más rápido que crearlo desde cero
- **Consistencia**: El lexer deserializado es funcionalmente idéntico al original
- **Portabilidad**: El archivo puede usarse en diferentes ejecuciones del programa

### Limitaciones
- **Compatibilidad**: Los archivos serializados dependen de la estructura del código. Si cambias la clase State, podrías necesitar re-serializar
- **Tamaño**: Para lexers muy complejos, el archivo puede ser grande
- **Memoria**: El lexer deserializado usa memoria dinámica que debe liberarse manualmente

## Manejo de Errores

El sistema incluye manejo de errores para:
- Archivos que no se pueden abrir
- Datos corruptos o incompletos
- Estados referenciados que no existen
- Errores de memoria

Siempre verifica el valor de retorno:

```cpp
// Para serialización
if (!lexer.serialize_lexer("archivo.bin")) {
    std::cerr << "Error al guardar" << std::endl;
}

// Para deserialización
Lexer* lexer = Lexer::deserialize_lexer("archivo.bin");
if (!lexer) {
    std::cerr << "Error al cargar" << std::endl;
}
```

## Pruebas

Ejecuta las pruebas de serialización para verificar que todo funciona:

```cpp
#include "test_serialization.h"

int main() {
    test_lexer_serialization();
    ejemplo_flujo_lexer_serializado();
    return 0;
}
```

## Notas Técnicas

- Los archivos se guardan en formato binario para eficiencia
- Se preservan todas las referencias entre estados
- El sistema maneja automáticamente la reconstrucción de punteros
- Compatible con autómatas de cualquier tamaño y complejidad

## Problemas Técnicos Resueltos

### 1. Error de Punteros en Serialización (CRÍTICO)
**Problema**: En `Lexer.h`, los métodos `serialize_lexer()` llamaban `automaton_.serialize_to_file()` pero `automaton_` es un puntero (`State*`), no un objeto.

**Solución**: Cambiar a `automaton_->serialize_to_file()` para usar correctamente la notación de punteros.

### 2. Pérdida de Tags durante Conversión DFA (CRÍTICO)
**Problema**: Cuando los NFAs se convertían a DFA usando `to_deterministic()`, los tags de los estados finales NFA no se copiaban a los estados finales DFA.

**Solución**: Modificar el método `to_deterministic()` en `state.cpp` para:
- Identificar estados finales en cada conjunto de estados NFA
- Asignar el tag del estado con menor número `n` (mayor prioridad) al estado DFA final
- Conservar esta información durante la serialización/deserialización

### 3. Manejo Robusto de Estados NFA en Tokenización
**Problema**: Los estados deserializados no tenían información válida de `nfa_states`, causando acceso a punteros nulos.

**Solución**: Modificar `tokenize()` en `Lexer.h` para:
- Usar directamente el tag del estado DFA final cuando esté disponible
- Solo recurrir a los estados NFA como fallback para compatibilidad
- Verificar punteros nulos antes de acceder a estados NFA

### 4. Gestión de Memoria
**Problema**: El Lexer no gestionaba correctamente la memoria de autómatas deserializados.

**Solución**: 
- Refactorizar `Lexer` para usar `State* automaton_` en lugar de `State automaton_`
- Añadir flag `owns_automaton_` para controlar cuándo liberar memoria
- Implementar destructor que limpia correctamente todos los estados
