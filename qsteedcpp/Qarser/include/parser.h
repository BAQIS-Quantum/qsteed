#pragma once
#include <memory>
#include <filesystem>
#include <vector>
#include <optional>
#include "lexer.h"
#include "AST/ast.hpp"
#include "AST/gate.hpp"

namespace qsteedcpp {
namespace qarser {

class Parser {
private:
    QasmLexer lexer;
    Token current;
    Token previous;
    
    std::vector<std::filesystem::path> include_paths_;
    std::filesystem::path current_file_path_;

public:
    Parser(const std::string& source);
    Parser();

    void add_include_path(const std::filesystem::path& path) {
        include_paths_.push_back(path);
    }
    
    void set_current_file(const std::filesystem::path& path) {
        current_file_path_ = path;
    }

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
    
    // Preprocess
    std::optional<std::filesystem::path> find_include_file(const std::string& filename);
    std::unique_ptr<Program> expand_include(const std::string& filename);
    void preprocess_statements(std::vector<std::unique_ptr<Statement>>& statements);
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
}; // namespace qsteedcpp