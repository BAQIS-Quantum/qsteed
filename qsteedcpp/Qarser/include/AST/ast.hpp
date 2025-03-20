#pragma once
#include <vector>
#include <string>
#include "visitor.hpp"
#include "SA/context/symbol.hpp"


namespace qarser {

    class AstNode {
    public:
        int line;

        AstNode(int line = 0) : line(line) {}
        virtual ~AstNode() = default;
        virtual void accept(AstVisitor& visitor) = 0;
    };


    class Statement : public AstNode {
    public: 
        enum class Kind {
            INCLUDE,
            QREG,
            CREG,
            GATE,
            GATE_DEF,
            MEASURE,
            BARRIER
        };

        Statement(int line = 0) : AstNode(line) {}
        virtual ~Statement() = default;

        virtual void accept(AstVisitor& visitor) = 0;
        virtual Kind kind() const = 0;
    };


    class Program : public AstNode {
    public:
        double version;
        std::vector<std::unique_ptr<Statement>> statements;

    public:  
        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }
    };

    class Include : public Statement {
    public:
        std::string filename;
    public:
        Include(const std::string& filename) 
            : filename(filename) {}

        void accept(AstVisitor& visitor) override { 
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::INCLUDE;
        }

    };


    // Register Declaration
    class Register : public Statement {
    public:
        std::string name;
        int size;

    protected:
        Register(int line, const std::string& name, int size) 
            : Statement(line), name(name), size(size) {}
       
        virtual void accept(AstVisitor& visitor) = 0;

    };


    class QRegister : public Register {
    public:
        QRegister(int line, const std::string& name, int size) 
            : Register(line, name, size) {}
     
        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::QREG;
        }
    };


    class CRegister : public Register {
    public:
        CRegister(int line, const std::string& name, int size) 
            : Register(line, name, size) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::CREG;
        }
    };


    class Reset : public Statement {
    public:
        std::string qubit;

    public:
        Reset(const std::string& qubit) 
            : qubit(qubit) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::MEASURE;
        }
    };


;

// TODO
// class If : public Statement {
// public:
//     std::string creg;
//     std::string value;
//     std::unique_ptr<Statement> statement;
// };



}; // namespace qarser