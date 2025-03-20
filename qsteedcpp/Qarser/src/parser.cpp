#include <iostream>
#include <cmath>
#include "parser.h"
#include "AST/ast.hpp"


namespace qarser {
    
    // -- Public :
    Parser::Parser(const std::string& source) 
        : lexer(source) {
        advance();
    }

    std::unique_ptr<Program> Parser::parse() {
        auto program = std::make_unique<Program>();

        parse_version(program);

        while (!match(TokenType::EOF_TOKEN)) {
            program->statements.push_back(parse_statement());
        }

        return program;
    }


    // -- Private :
    void Parser::advance() {
        previous = current;
        current = lexer.next();
    }

    Token Parser::consume(TokenType type, const std::string& message) {
        if (current.type == type) {
            Token temp = current; 
            advance();
            return temp;
        }
        error(message);
    }

    bool Parser::try_consume(TokenType type) {
        if (match(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool Parser::match(TokenType type) {
        return type == current.type;
    }

    void Parser::error(const std::string& message) {
        throw ParsingError(current.line, current.column, message, current.lexeme);
    }


    void Parser::parse_version(std::unique_ptr<Program>& program) {
        consume(TokenType::OPENQASM, "Expect OPENQASM key word!");
        Token version = consume(TokenType::NUMBER, "Expect Version number!");
        double version_num = std::stod(version.lexeme);
        if (version_num != 2.0) {
            throw std::runtime_error("Only support OPENQASM 2.0!");
        }
        program->version = version_num;
        consume(TokenType::SEMICOLON, "Expect Semicolon!");
    } 


    std::unique_ptr<Statement> Parser::parse_statement() {
        if (match(TokenType::INCLUDE)) return parse_include();
        if (match(TokenType::QREG)) return parse_qreg();
        if (match(TokenType::CREG)) return parse_creg();
        if (match(TokenType::MEASURE)) return parse_measure();
        if (match(TokenType::BARRIER)) return parse_barrier();

        if (match(TokenType::GATE)) 
            return parse_gate_def();

        return parse_gate();
    }

    std::unique_ptr<Include> Parser::parse_include() {
        consume(TokenType::INCLUDE, "Expect include key word!");
        Token filename = consume(TokenType::STRING, "Expect filename!");
        consume(TokenType::SEMICOLON, "Expect ';' !");

        return std::make_unique<Include>(filename.lexeme);
    }

    std::unique_ptr<QRegister> Parser::parse_qreg() {
        consume(TokenType::QREG, "Expect qreg key word!");
        auto [name, size] = parse_register_declaration();
        consume(TokenType::SEMICOLON, "Expect ';' while parsing qreg!");
        return std::make_unique<QRegister>(previous.line, name, size);
    }

    std::unique_ptr<CRegister> Parser::parse_creg() {
        consume(TokenType::CREG, "Expect creg key word!");
        auto [name, size] = parse_register_declaration();
        consume(TokenType::SEMICOLON, "Expect ';' while parsing creg!");
        return std::make_unique<CRegister>(previous.line, name, size);
    }


    std::pair<std::string, int> Parser::parse_register_declaration() {
        Token name = consume(TokenType::IDENTIFIER, "Expect register name!");
        consume(TokenType::LEFT_BRACKET, "Parsing register declaration, Expect '[' !");
        Token size = consume(TokenType::NUMBER, "Expect register size!");
        consume(TokenType::RIGHT_BRACKET, "Expect Right Bracket ']' !");

        return {name.lexeme, std::stoi(size.lexeme)};
    }
    
    RegisterRef Parser::parse_single_register_ref() {
        Token reg = consume(TokenType::IDENTIFIER, "Expect register name!");
        if (try_consume(TokenType::LEFT_BRACKET)) {
            Token index = consume(TokenType::NUMBER, "Expect index!");
            consume(TokenType::RIGHT_BRACKET, "Expect ']'!");
            return RegisterRef(reg.lexeme, std::stoi(index.lexeme));
        }
        else
            return RegisterRef(reg.lexeme);
    }

    std::vector<RegisterRef> Parser::parse_register_ref() {
        std::vector<RegisterRef> refs;
        do {
            refs.push_back(parse_single_register_ref());
        } while (try_consume(TokenType::COMMA));

        return refs;
    }

    std::unique_ptr<Gate> Parser::parse_gate() {
        Token name = consume(TokenType::IDENTIFIER, "Expect a gate name!");

        // Parsing gate parameters
        std::vector<std::unique_ptr<Expression>> parameters;
        if (try_consume(TokenType::LEFT_PAREN)) {
            do {
                parameters.push_back(parse_expression());
            } while (try_consume(TokenType::COMMA));
            consume(TokenType::RIGHT_PAREN, "Expect ')' !");
        }
               
        // Parsing qreg references
        std::vector<RegisterRef> qubits = parse_register_ref();

        consume(TokenType::SEMICOLON, "Expect ';'");
        return std::make_unique<Gate>(
            name.line,
            name.lexeme,
            std::move(parameters),
            qubits
        );
    }


    std::unique_ptr<GateDef> Parser::parse_gate_def() {
        consume(TokenType::GATE, "Expect gate key word!");
        Token name = consume(TokenType::IDENTIFIER, "Expect gate name!");

        // Parsing gate parameters
        std::vector<std::string> parameters;
        if (try_consume(TokenType::LEFT_PAREN)) {
            do {
                Token param = consume(TokenType::IDENTIFIER, "Expect parameter name!");
                parameters.push_back(param.lexeme);
            } while (try_consume(TokenType::COMMA));
            consume(TokenType::RIGHT_PAREN, "Expect ')' !");
        }

        std::vector<RegisterRef> qubits = parse_register_ref();

        std::vector<std::unique_ptr<Statement>> body;
        consume(TokenType::LEFT_BRACE, "Expect '{' !");
        while (!match(TokenType::RIGHT_BRACE)) {
            body.push_back(parse_gate_def_body());
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' !");

        return std::make_unique<GateDef>(
            name.line,
            name.lexeme,
            std::move(parameters),
            qubits,
            std::move(body)
        );
    }

    std::unique_ptr<Statement> Parser::parse_gate_def_body() {
        if (match(TokenType::BARRIER)) 
            return parse_barrier();
        else if(match(TokenType::IDENTIFIER)){
            return parse_gate();
        }
        else {
            error("Expect gate or barrier in gate definition body!");
        }
    }


    std::unique_ptr<Expression> Parser::parse_expression() {
        return parse_additive();
    }

    std::unique_ptr<Expression> Parser::parse_additive() {
        auto left = parse_multiplicative();
       
        while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
            Token op = current;
            advance();

            auto right = parse_multiplicative();

            left = std::make_unique<BinaryExpr>(
                op.line, op.type,
                std::move(left),
                std::move(right)
            );
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parse_multiplicative() {
        std::unique_ptr<Expression> left = parse_unary();

        while (match(TokenType::STAR) || match(TokenType::SLASH)) {
            Token op = current;
            advance();
            std::unique_ptr<Expression> right = parse_unary();

            left = std::make_unique<BinaryExpr>(
                op.line, op.type, 
                std::move(left), 
                std::move(right)
            );
        }
        return left;
    }


    std::unique_ptr<Expression> Parser::parse_unary() {
        if (match(TokenType::MINUS) || match(TokenType::PLUS)) {
            Token op = current;
            advance();
            auto operand = parse_primary();

            return std::make_unique<UnaryExpr>(
                op.line,
                op.type,
                std::move(operand)
            );
        }

        if ( match(TokenType::SIN) || match(TokenType::COS) ||
             match(TokenType::TAN) || match(TokenType::EXP) ||
             match(TokenType::LN) ) {

            Token op = current;
            advance();

            consume(TokenType::LEFT_PAREN, "Expect '(' !");
            auto operand = parse_expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' !");

            return std::make_unique<UnaryExpr>(
                op.line,
                op.type,
                std::move(operand)
            );
        }

        return parse_primary();
    }



    std::unique_ptr<Expression> Parser::parse_primary() {
        if (try_consume(TokenType::NUMBER)) {
            return std::make_unique<NumberExpr>(
                previous.line, 
                std::stod(previous.lexeme)
            );
        }

        // Identifier
        if (try_consume(TokenType::IDENTIFIER)) {
            if (previous.lexeme == "pi") {
                return std::make_unique<NumberExpr>(
                    previous.line,
                    M_PI
                );
            }
            if (previous.lexeme == "e") {
                return std::make_unique<NumberExpr>(
                    previous.line,
                    M_E
                );
            }
            return std::make_unique<IdentifierExpr>(
                previous.line,
                previous.lexeme
            );
        }


        if (try_consume(TokenType::LEFT_PAREN)) {
            std::unique_ptr<Expression> expr = parse_expression();
            consume(TokenType::RIGHT_PAREN, "Expect ')' !");
            return expr;
        }

        error("Expect expression!");
    }




    std::unique_ptr<Measure> Parser::parse_measure() {
        consume(TokenType::MEASURE, "Expect measure key word!");

        std::vector<RegisterRef> qubits = parse_register_ref();
        Token arrow = consume(TokenType::ARROW, "Expect right arrow '->' !");
        std::vector<RegisterRef> cbits = parse_register_ref();

        consume(TokenType::SEMICOLON, "Expect ';'!");

        return std::make_unique<Measure>(previous.line, qubits, cbits);
    }

    std::unique_ptr<Barrier> Parser::parse_barrier() {
        consume(TokenType::BARRIER, "Expect barrier key word!");
        std::vector<RegisterRef> qubits = parse_register_ref();
        consume(TokenType::SEMICOLON, "Parsing barrier, Expect ';'!");

        return std::make_unique<Barrier>(previous.line, qubits);
    }

} // namespace qarser
