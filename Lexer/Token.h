#pragma once

#include <string>
#include "../Grammar/symbol.h"

/**
 * @class Token
 * @brief Representa un token con información de posición en el código fuente.
 * 
 * Extiende la funcionalidad del Terminal para incluir información de línea y columna,
 * permitiendo generar mensajes de error más precisos.
 */
class Token : public Terminal {
public:
    /**
     * @brief Constructor del token con información de posición.
     * @param name Nombre del token (tipo)
     * @param lexeme Texto exacto del token
     * @param grammar Gramática asociada
     * @param line Línea donde comienza el token (1-based)
     * @param column Columna donde comienza el token (1-based)
     */
    Token(const std::string& name, const std::string& lexeme, Grammar& grammar, int line = 1, int column = 1)
        : Terminal(name, grammar), lexeme_(lexeme), line_(line), column_(column) {}
    
    /**
     * @brief Obtiene el lexema (texto) del token.
     * @return Texto del token
     */
    const std::string& Lexeme() const { return lexeme_; }
    
    /**
     * @brief Obtiene la línea donde comienza el token.
     * @return Número de línea (1-based)
     */
    int Line() const { return line_; }
    
    /**
     * @brief Obtiene la columna donde comienza el token.
     * @return Número de columna (1-based)
     */
    int Column() const { return column_; }
    
    /**
     * @brief Representación string del token incluyendo posición.
     * @return String con formato: "tipo[linea,columna]:lexema"
     */
    std::string ToString() const {
        return Name() + "[" + std::to_string(line_) + "," + 
               std::to_string(column_) + "]:" + lexeme_;
    }

    /**
     * @brief Compara dos tokens por su lexema, línea y columna.
     * @param other Otro token a comparar
     * @return true si son iguales, false en caso contrario
     */
    bool operator==(const Token& other) const {
        return lexeme_ == other.lexeme_ && line_ == other.line_ && column_ == other.column_ && name == other.name;
    }

private:
    std::string lexeme_;
    int line_;
    int column_;
};
