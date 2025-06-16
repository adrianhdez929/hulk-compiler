# Serialización del Parser SLR1

## Descripción

Este sistema permite guardar (serializar) y cargar (deserializar) un Parser SLR1 completo en un archivo binario. Esto complementa la funcionalidad de serialización del Lexer y es útil para:

- **Acelerar el inicio del compilador**: Una vez que tienes tu parser configurado, puedes guardarlo y cargarlo rápidamente en futuras ejecuciones
- **Distribuir parsers precompilados**: Puedes crear parsers y distribuirlos sin necesidad de reconstruir las tablas de parsing
- **Cachear configuraciones complejas**: Para gramáticas grandes y complejas, la serialización evita reconstruir todas las tablas LR0/SLR1

## Uso Básico

### Serialización (Guardar)

```cpp
// Crear parser original
Grammar g = GrammarParser::Parse("mi_gramatica.txt");
SLR1Parser parser(g);

// Serializar en carpeta hulk (por defecto)
bool success = parser.serialize_parser("mi_parser.slr");

// O especificar carpeta personalizada
bool success = parser.serialize_parser("mi_parser.slr", "mi_carpeta");
```

### Deserialización (Cargar)

```cpp
// Cargar desde carpeta hulk (por defecto)
Grammar g = GrammarParser::Parse("mi_gramatica.txt");
SLR1Parser* parser = SLR1Parser::deserialize_parser("mi_parser.slr", g);

// O especificar carpeta personalizada
SLR1Parser* parser = SLR1Parser::deserialize_parser("mi_parser.slr", "mi_carpeta", g);

// Usar el parser
auto [productions, actions] = parser->Parse(tokens);

// Limpiar memoria
delete parser;
```

## Componentes Serializados

El sistema serializa los siguientes componentes críticos del parser:

1. **Tabla Action**: Mapeo de `(estado, terminal) -> (acción, valor)`
   - Acciones: SHIFT, REDUCE, OK
   - Estados de destino y números de producción

2. **Tabla Goto**: Mapeo de `(estado, no_terminal) -> estado_destino`
   - Transiciones entre estados para símbolos no terminales

3. **Configuración del Parser**: Flags como `verbose`

4. **Metadatos**: Versión del formato, firma del archivo

## Formato del Archivo

- **Extensión recomendada**: `.slr`
- **Formato**: Binario
- **Firma**: "SLR1PARSER" (10 bytes)
- **Versión**: uint32_t (actualmente versión 1)
- **Contenido**: Tablas serializadas con información de tipos

## Gestión de Directorios

Al igual que el Lexer, el sistema automáticamente:

- ✅ Crea la carpeta `hulk/` si no existe
- ✅ Crea carpetas personalizadas si se especifican
- ✅ Proporciona mensajes informativos sobre la ubicación de archivos

## Ejemplo Completo

```cpp
#include "Parser/SLR1Parser.h"
#include "Lexer/grammar_parser.h"

int main() {
    try {
        // Cargar gramática
        Grammar g = GrammarParser::Parse("mi_gramatica.txt");
        
        // Crear parser original
        SLR1Parser parser_original(g);
        
        // Probar funcionamiento original
        std::vector<std::string> tokens = {"symbol", "EOF"};
        auto [prod_orig, actions_orig] = parser_original.Parse(tokens);
        
        // Serializar
        if (!parser_original.serialize_parser("test.slr")) {
            std::cerr << "Error en serialización" << std::endl;
            return 1;
        }
        
        // Deserializar
        auto parser_new = SLR1Parser::deserialize_parser("test.slr", g);
        if (!parser_new) {
            std::cerr << "Error en deserialización" << std::endl;
            return 1;
        }
        
        // Probar funcionamiento deserializado
        auto [prod_new, actions_new] = parser_new->Parse(tokens);
        
        // Verificar que son idénticos
        assert(prod_orig == prod_new);
        assert(actions_orig == actions_new);
        
        delete parser_new;
        std::cout << "¡Parser serializado/deserializado funciona perfectamente!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## Consideraciones Importantes

### Dependencia de la Gramática
⚠️ **IMPORTANTE**: El parser deserializado necesita una referencia a la misma gramática que se usó originalmente. Esto es porque:

- Los símbolos se serializan por nombre, no por puntero
- La gramática debe estar disponible para reconstruir las referencias a símbolos
- Se recomienda serializar la gramática por separado si es necesario

### Compatibilidad de Versiones
- El sistema verifica la versión del archivo deserializado
- Archivos de versiones no compatibles son rechazados con mensaje de error
- La versión actual es 1

### Manejo de Errores
El sistema proporciona mensajes detallados para:
- Archivos no encontrados
- Formatos de archivo inválidos
- Versiones incompatibles
- Símbolos no encontrados en la gramática
- Errores de E/O del sistema de archivos

## Archivos de Prueba

Incluidos en el proyecto:

- `test_parser_serialization.cpp`: Test básico de serialización del parser
- `test_combined_serialization.cpp`: Test que verifica parser + lexer juntos

## Compatibilidad con Lexer

Este sistema de serialización del parser es **100% compatible** con el sistema de serialización del Lexer implementado previamente. Puedes:

- Usar ambos sistemas independientemente
- Serializar y deserializar lexer y parser por separado
- Combinar ambos en un pipeline completo de compilación
- Todos los archivos se guardan en la misma carpeta `hulk/` por defecto

## Estado del Proyecto

✅ **COMPLETAMENTE IMPLEMENTADO Y PROBADO**
- Serialización completa del parser SLR1
- Deserialización robusta con validación
- Gestión automática de directorios
- Manejo completo de errores
- Tests exhaustivos que verifican funcionamiento idéntico
- Compatibilidad total con sistema de Lexer
- Documentación completa

El sistema está listo para uso en producción.
