#pragma once
#include <memory>
#include "lexer.h"
#include "AST/ast.hpp"
#include "AST/gate.hpp"

namespace qarser {

class Parser {
private:
    QasmLexer lexer;
    Token current;
    Token previous;

public:
    Parser(const std::string& source);
    std::unique_ptr<Program> parse();

private:
    void advance();

    Token consume(TokenType type, const std::string& message);

    bool try_consume(TokenType type);

    bool match(TokenType type);

    [[noreturn]] void error(const std::string& message);



    void parse_version(std::unique_ptr<Program>& program);

    std::unique_ptr<Statement> parse_statement();
    std::unique_ptr<Include> parse_include();
    std::unique_ptr<QRegister> parse_qreg();
    std::unique_ptr<CRegister> parse_creg();
    std::unique_ptr<Measure> parse_measure();
    std::unique_ptr<Barrier> parse_barrier();
    // std::unique_ptr<Reset> parse_reset();
    // std::unique_ptr<If> parse_if();

    std::unique_ptr<Gate> parse_gate();
    std::unique_ptr<GateDef> parse_gate_def();
    std::unique_ptr<Statement> parse_gate_def_body();

    // Experssion parsing
    std::unique_ptr<Expression> parse_expression();
    std::unique_ptr<Expression> parse_additive();
    std::unique_ptr<Expression> parse_multiplicative();
    std::unique_ptr<Expression> parse_unary();
    std::unique_ptr<Expression> parse_primary();



    std::pair<std::string, int> parse_register_declaration();
    RegisterRef parse_single_register_ref();
    std::vector<RegisterRef> parse_register_ref();
};



class ParsingError : public std::runtime_error {
public:
    int line;
    int column;

public:
    ParsingError(int line, int column, const std::string& message) 
        : std::runtime_error(
            "Error at line " + std::to_string(line) + 
            " column " + std::to_string(column) + ": " + message
        ),
        line(line),
        column(column) {}


    ParsingError(int line, int column, const std::string& message, const std::string& lexeme)
        : std::runtime_error(
            "Error at line " + std::to_string(line) + 
            " column " + std::to_string(column) + ": " + message
            + " Found: " + lexeme
        ),
        line(line),
        column(column) {}
};



}; // namespace qarser