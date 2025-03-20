#pragma once
#include "ast.hpp"


namespace qarser {

    class Expression : public AstNode {
    public:
        Expression(int line = 0) : AstNode(line) {}
        virtual ~Expression() = default;
        virtual void accept(AstVisitor& visitor) = 0;
    };


    class NumberExpr : public Expression {
    public:
        double value;

    public:
        NumberExpr(int line, double value) 
            : Expression(line), value(value) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }
    };


    // Eg. (pi, theta)
    class IdentifierExpr : public Expression {
    public:
        std::string name;

        IdentifierExpr(int line, const std::string& name) 
            : Expression(line), name(name) {}
        
        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }
    };


    // Eg. (-, sin, cos, tan, exp, ln)
    class UnaryExpr : public Expression {
    public:
        enum class Op { Neg, Pos ,Sin, Cos, Tan, Exp, Ln };

        static Op token_to_op(TokenType type) {
            switch (type) {
                case TokenType::MINUS: return Op::Neg;
                case TokenType::PLUS: return Op::Pos;
                case TokenType::SIN: return Op::Sin;
                case TokenType::COS: return Op::Cos;
                case TokenType::TAN: return Op::Tan;
                case TokenType::EXP: return Op::Exp;
                case TokenType::LN: return Op::Ln;

                default: 
                    throw std::runtime_error("Invalid Unary Operator!");
            }

        }

        Op op;
        std::unique_ptr<Expression> operand;

        UnaryExpr(int line, TokenType type, std::unique_ptr<Expression> operand) 
            : Expression(line), op(token_to_op(type)), operand(std::move(operand)) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }
    };


    class BinaryExpr : public Expression {
    public:
        enum class Op { Add, Sub, Mul, Div };

        static Op token_to_op(TokenType type) {
            switch (type) {
                case TokenType::PLUS:   return Op::Add;
                case TokenType::MINUS:  return Op::Sub;
                case TokenType::STAR:   return Op::Mul;
                case TokenType::SLASH:  return Op::Div;

                default: 
                    throw std::runtime_error("Invalid Binary Operator!");
            }
        }


        Op op;
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;

        BinaryExpr(int line, TokenType type, 
                    std::unique_ptr<Expression> left, 
                    std::unique_ptr<Expression> right
        ) : Expression(line), op(token_to_op(type)), left(std::move(left)), right(std::move(right)) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }


    };







};