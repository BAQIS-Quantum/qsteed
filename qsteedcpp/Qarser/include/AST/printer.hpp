#pragma once
#include <iostream>
#include "ast.hpp"

namespace qarser {

class AstPrinter: public BaseVisitor {
public:
    int indent = 1;

private:
    void print_indent() {
        for (int i = 0; i < indent; i++) {
            std::cout << "  ";
        }
    }

public:
    void visit(Program& program) override {
        std::cout << "Program(version=" << program.version << ")\n";
        for (const auto& statement : program.statements) {
            print_indent();
            statement->accept(*this);
        }
    }

    void visit(Include& include) override {
        std::cout << "Include(filename=" << include.filename << ")\n";
    }

    void visit(QRegister& qreg) override {
        std::cout << "QReg(name=" << qreg.name 
            << ", size=" << qreg.size << ")\n";
    }

    void visit(CRegister& creg) override {
        std::cout << "CReg(name=" << creg.name 
            << ", size=" << creg.size << ")\n";
    }

    void visit(Gate& gate) override {
        std::cout << "Gate(name=" << gate.name;

        if (gate.params.empty()) {
            std::cout << ", params=[]";
        }
        if (!gate.params.empty()) {
            std::cout << ", params=[";
            for (size_t i = 0; i < gate.params.size(); ++i) {
                if (i > 0) std::cout << ", ";
                gate.params[i]->accept(*this);
            }
            std::cout << "]";
        }

        std::cout << ", qubits=";
        for (const auto& qubit : gate.qubits) {
            std::cout << qubit.toString();
        }
        std::cout << ")\n";
    }


    void visit(GateDef& gate_def) override {
        std::cout << "GateDef(name=" << gate_def.name;
    
        // print gate parameters
        if (gate_def.params.empty()) {
            std::cout << ", params=[]";
        } else {
            std::cout << ", params=[";
            for (size_t i = 0; i < gate_def.params.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << gate_def.params[i];
            }
            std::cout << "]";
        }
        
        // print qubits
        std::cout << ", qubits=";
        for (size_t i = 0; i < gate_def.qubits.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << gate_def.qubits[i].toString();
        }
        
        std::cout << ")\n";
        
        // 打印门定义体中的语句
        indent++; // 增加缩进
        for (const auto& statement : gate_def.body) {
            print_indent();
            statement->accept(*this);
        }
        indent--; // 恢复缩进
    }


    void visit(Measure& measure) override {
        std::cout << "Measure(qubit=";
        for (const auto& qubit : measure.qubits) {
            std::cout << qubit.toString();
        }
        std::cout << ", creg=";
        for (const auto& qubit : measure.qubits) {
            std::cout << qubit.toString();
        }
        std::cout << ")\n";
    }

    void visit(Barrier& barrier) override {
        std::cout << "Barrier(qubits=";
        for (const auto& qubit : barrier.qubits) {
            std::cout << qubit.toString();
        }
        std::cout << ")\n"; 
    }

    void visit(Reset& reset) override {

    }




    // Expressions
    void visit(NumberExpr& expr) override {
        std::cout << "Number(" << expr.value << ")";
    }
    
    void visit(IdentifierExpr& expr) override {
        std::cout << "Identifier(" << expr.name << ")";
    }


    void visit(UnaryExpr& expr) override {
        std::cout << "Unary(op=";
        switch (expr.op) {
            case UnaryExpr::Op::Neg:
                std::cout << "-";
                break;
            case UnaryExpr::Op::Pos:
                std::cout << "+";
                break;
            case UnaryExpr::Op::Sin:
                std::cout << "sin";
                break;
            case UnaryExpr::Op::Cos:
                std::cout << "cos";
                break;
            case UnaryExpr::Op::Tan:
                std::cout << "tan";
                break;
            case UnaryExpr::Op::Exp:
                std::cout << "exp";
                break;
            case UnaryExpr::Op::Ln:
                std::cout << "ln";
                break;
           default:
                std::cout << "unknown";
        }
        std::cout << ", operand=";
        expr.operand->accept(*this);
        std::cout << ")";
    }


    void visit(BinaryExpr& expr) override {
        std::cout << "Binary(op=";
        switch (expr.op) {
            case BinaryExpr::Op::Add:
                std::cout << "+";
                break;
            case BinaryExpr::Op::Sub:
                std::cout << "-";
                break;
            case BinaryExpr::Op::Mul:
                std::cout << "*";
                break;
            case BinaryExpr::Op::Div:
                std::cout << "/";
                break;
           default:
                std::cout << "unknown";
        }
        std::cout << ", l=";
        expr.left->accept(*this);
        std::cout << ", r=";
        expr.right->accept(*this);
        std::cout << ")";
    }

   
};




};