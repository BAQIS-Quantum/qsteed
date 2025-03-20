#include <cctype>
#include <stdexcept> 
#include "lexer.h"

namespace qarser {

QasmLexer::QasmLexer(const std::string& source) 
    : source(source) {}


Token QasmLexer::next() {
    skip_whitespace();
    if (position >= source.length()) {
        return Token{TokenType::EOF_TOKEN, "", line, column};
    }

    char c = peek();


    // Check for Identifier
    if (isalpha(c)) {
        std::string identifier;
        while (isalnum(peek()) || peek() == '_') {
            identifier += advance();
        }

        auto it = keywords.find(identifier);
        if (it != keywords.end()) {
            return Token{it->second, identifier, line, column};
        }    
        return Token{TokenType::IDENTIFIER, identifier, line, column};
    }

    // Check for Number
    if (isdigit(c)) {
        std::string number;
        while (isdigit(peek())) {
            number += advance();
        }
        
        if (peek() == '.') {
            number += advance();
            while (isdigit(peek())) {
                number += advance();
            }
        }
        return Token{TokenType::NUMBER, number, line, column};
    }

    // Check for String
    if (c == '"') {
        std::string str;
        advance();
        while (peek() != '"') {
            if (peek() == '\0') {
                throw std::runtime_error("Unterminated string");
            }
            str += advance();
        }
        advance();
        return Token{TokenType::STRING, str, line, column};
    }



    // Check for Symbol
    switch (advance()) {
        case '{': return Token{TokenType::LEFT_BRACE,   "{", line, column};
        case '}': return Token{TokenType::RIGHT_BRACE,  "}", line, column};
        case '[': return Token{TokenType::LEFT_BRACKET, "[", line, column};
        case ']': return Token{TokenType::RIGHT_BRACKET, "]", line, column};
        case '(': return Token{TokenType::LEFT_PAREN,   "(", line, column};
        case ')': return Token{TokenType::RIGHT_PAREN,  ")", line, column};
        case ';': return Token{TokenType::SEMICOLON,    ";", line, column};
        case ',': return Token{TokenType::COMMA,        ",", line, column};
        case '*': return Token{TokenType::STAR,         "*", line, column};
        case '/': return Token{TokenType::SLASH,        "/", line, column};
        case '+': return Token{TokenType::PLUS,         "+", line, column};
        case '-':
            if (peek() == '>') {
                advance();
                return Token{TokenType::ARROW, "->", line, column};
            }
            else {
                return Token{TokenType::MINUS, "-", line, column};
            }
            
            break;

        default:
            std::string error = "Unexpected character: ";
            error += peek();
            throw std::runtime_error(error);
    }


    return Token{TokenType::ERROR, "error", line, column};
}




char QasmLexer::peek() const {
    if (position >= source.length()) {
        return '\0';
    }
    return source[position];
}

char QasmLexer::advance() {
    column++;
    if (position >= source.length()) {
        return '\0';
    }
    return source[position++];
}

void QasmLexer::skip_whitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();        
                break;

            case '\n':
                line++;
                column = 1;
                advance();
                break;


            case '/':
                if (position + 1 >= source.length()) {
                    return;
                }
                
                if (source[position + 1] == '/') {
                    // 单行注释
                    advance(); // '/'
                    advance(); // '/'
                    while (!is_at_end() && peek() != '\n') {
                        advance();
                    }
                    continue;
                }
                
                if (source[position + 1] == '*') {
                    // 多行注释
                    advance(); // '/'
                    advance(); // '*'
                    while (!is_at_end()) {
                        if (peek() == '*' && position + 1 < source.length() && 
                            source[position + 1] == '/') {
                            advance(); // '*'
                            advance(); // '/'
                            break;
                        }
                        if (peek() == '\n') {
                            line++;
                            column = 1;
                        }
                        advance();
                    }
                    continue;
                }
                return;            

            default:
                return;
        }
    }
}


bool QasmLexer::is_at_end() const {
    return position >= source.length();
}


// Alaphabet and Keywords
const std::unordered_map<std::string, TokenType> QasmLexer::keywords {
    {"OPENQASM",  TokenType::OPENQASM},
    {"include",   TokenType::INCLUDE},
    
    {"qreg",      TokenType::QREG},
    {"creg",      TokenType::CREG},
    
    {"measure",   TokenType::MEASURE},
    {"barrier",   TokenType::BARRIER},
    {"reset",     TokenType::RESET},

    {"gate",      TokenType::GATE}, // gate definition key word.
    
    {"if",        TokenType::IF},
       
    {"sin",       TokenType::SIN},
    {"cos",       TokenType::COS},
    {"exp",       TokenType::EXP},
    {"ln",        TokenType::LN}
};

};