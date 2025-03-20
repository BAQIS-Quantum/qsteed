#pragma once
#include "ast.hpp"
#include "expression.hpp"

namespace qarser {

    class RegisterRef {
    public:
        std::string name;
        int index;

    public:
        RegisterRef(const std::string& name) 
            : name(name), index(-1) {}

        RegisterRef(const std::string& name, int index) 
            : name(name), index(index) {}

        bool isRefWholeRegister() const {
            return index == -1;
        }

        std::string toString() const {
            if (isRefWholeRegister()) {
                return name;
            }
            return name + "[" + std::to_string(index) + "]";
        }
    };


    class Gate : public Statement {
    public:
        std::string name;
        std::vector<RegisterRef> qubits;
        std::vector<std::unique_ptr<Expression>> params;
    public:
        Gate(int line, 
            const std::string& name, 
            std::vector<std::unique_ptr<Expression>>&& params,
            const std::vector<RegisterRef>& qubits
        )
            : Statement(line), 
                name(name), 
                params(std::move(params)),
                qubits(qubits) {}


        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::GATE;
        }
    };


    class GateDef : public Statement {
    public:
        std::string name;
        std::vector<RegisterRef> qubits;
        std::vector<std::string> params;
        std::vector<std::unique_ptr<Statement>> body;

    public:
        GateDef(int line, 
            const std::string& name, 
            std::vector<std::string>&& params,
            const std::vector<RegisterRef>& qubits,
            std::vector<std::unique_ptr<Statement>>&& body
        )
            : Statement(line), 
                name(name), 
                params(std::move(params)),
                qubits(qubits),
                body(std::move(body)) {}


        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::GATE_DEF;
        }
    };



    class Measure : public Statement {
    public:
        std::vector<RegisterRef> qubits;
        std::vector<RegisterRef> cbits;
    public:
        Measure(int line, const std::vector<RegisterRef>& qubits, const std::vector<RegisterRef>& cbits) 
            : Statement(line), qubits(qubits), cbits(cbits) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::MEASURE;
        }
    };


    class Barrier : public Statement {
    public:
        std::vector<RegisterRef> qubits;
    public:
        Barrier(int line, const std::vector<RegisterRef>& qubits) 
            : Statement(line), qubits(qubits) {}

        void accept(AstVisitor& visitor) override {
            visitor.visit(*this);
        }

        Kind kind() const override {
            return Kind::BARRIER;
        }
    };


};


