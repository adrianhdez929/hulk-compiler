# Como usar la API de gramatica

## Instanciar la clase `Grammar`

`Grammar g;`

## Crear no terminales y terinales

1. No terminal: `auto T = g.SetNonTerminal("T");`
2. Terminal: `auto plus = g.SetTerminal("+");`

## Para establecer un simbolo como simbolo inicial de la gramatica

`auto E = g.SetNonTerminal("E", true);`

## Crear producciones

1. Parte derecha con mas de un simbolo:

```python
    E %= {
        E + plus + T,
        {
            // Acción para E (heredado, sintetizado)
            [](auto h, auto s) { 
                //function body
            },
            // Acción para '+'
            [](auto h, auto s) { //function body } ,
            // Acción para T
            [](auto h, auto s) { //function body },
            // Acción para LHS (E)
            [](auto h, auto s) { //function body }
        }
    };
```

**Estructura**: `no terminal %= { simbolo + simbolo + ... + simbolo, {funcion lambda, funcion lambda, ..., funcion lambda}};`.
**Tiene** q haber la misma cantidad de simbolos en la produccion (incluyendo el no terminal antes de `%=`) que de funciones lambda.
2. Parte derecha con un solo simbolo:

```python
    E %= {T.GetSentence(),
        {
            [](auto h, auto s) { 
                //function body
            }
        }
    };
```

**Estructura**: `no terminal %= {simbolo.GetSentence(),{funcion lambda, funcion lambda}}`

## Aclaraciones generales:

1. Las funciones lambda **no** son ejecutadas por la clase `Grammar`, simplemente esta las almacena.
2. Las funciones lambda **no** estan asignadas ni agrupadas con ningun simbolo en `Grammar`. Se deja al programador la eleccion de cual usar para que simbolo.
