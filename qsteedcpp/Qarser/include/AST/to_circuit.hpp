#pragma once
#include <map>
#include <string>
#include <stdexcept>
#include "AST/visitor.hpp"
#include "AST/gate.hpp"
#include "AST/expression.hpp"
#include "AST/expression_converter.hpp"
#include "QuantumCircuit/include/circuit/quantum_circuit.h"
#include "QuantumCircuit/include/circuit/parameter.h"

namespace qsteedcpp{
namespace qarser {

    class AstToCircuit : public AstVisitor {
    private:
        std::unique_ptr<QuantumCircuit> qc;
        std::map<std::string, int> qubit_mapping;  // 寄存器名到起始索引的映射
        std::map<std::string, int> clbit_mapping;  // 经典寄存器映射
        int total_qubits = 0;
        int total_clbits = 0;
        
    public:
        AstToCircuit() = default;
        
        std::unique_ptr<QuantumCircuit> convert(Program& program) {
            // 首先遍历获取寄存器信息
            for (const auto& stmt : program.statements) {
                if (stmt->kind() == Statement::Kind::QREG) {
                    stmt->accept(*this);
                } else if (stmt->kind() == Statement::Kind::CREG) {
                    stmt->accept(*this);
                }
            }
            
            // 创建量子电路
            qc = std::make_unique<QuantumCircuit>(total_qubits, total_clbits);
            
            // 再次遍历处理门操作
            for (const auto& stmt : program.statements) {
                if (stmt->kind() == Statement::Kind::GATE || 
                    stmt->kind() == Statement::Kind::MEASURE ||
                    stmt->kind() == Statement::Kind::BARRIER) {
                    stmt->accept(*this);
                }
            }
            
            return std::move(qc);
        }
        
        // 访问器实现
        void visit(Program& node) override {
            // Program已在convert中处理
        }
        
        void visit(Include& node) override {
            // Include在SA阶段处理，这里忽略
        }
        
        void visit(QRegister& node) override {
            qubit_mapping[node.name] = total_qubits;
            total_qubits += node.size;
        }
        
        void visit(CRegister& node) override {
            clbit_mapping[node.name] = total_clbits;
            total_clbits += node.size;
        }
        
        void visit(Gate& node) override {
            // 展开寄存器引用
            auto qubits = expand_register_refs(node.qubits);
            
            // 转换参数
            std::vector<Parameter> params;
            for (const auto& expr : node.params) {
                params.push_back(expression_to_parameter(*expr));
            }
            
            // 使用门工厂创建门对象
            auto gate = GateFactory::create_gate(node.name, params);
            qc->add_gate(std::move(gate), qubits);
        }
        
        void visit(Measure& node) override {
            auto qubits = expand_register_refs(node.qubits);
            auto clbits = expand_register_refs_classical(node.cbits);
            
            if (qubits.size() != clbits.size()) {
                throw std::runtime_error("Measure: qubit and clbit count mismatch");
            }
            
            for (size_t i = 0; i < qubits.size(); ++i) {
                qc->measure(qubits[i], clbits[i]);
            }
        }
        
        void visit(Barrier& node) override {
            auto qubits = expand_register_refs(node.qubits);
            if (qubits.empty()) {
                qc->barrier();
            } else {
                qc->barrier(qubits);
            }
        }
        
        void visit(Reset& node) override {
            // TODO: 实现Reset
        }
        
        void visit(GateDef& node) override {
            // 自定义门定义，这里暂不处理
            // 可以考虑创建自定义门类
        }
        
        // Expression访问器（用于参数转换）
        void visit(Number& node) override {}
        void visit(Pi& node) override {}
        void visit(Identifier& node) override {}
        void visit(BinaryOp& node) override {}
        void visit(UnaryOp& node) override {}
        
    private:
        // 展开寄存器引用为量子比特索引
        std::vector<int> expand_register_refs(const std::vector<RegisterRef>& refs) {
            std::vector<int> result;
            for (const auto& ref : refs) {
                auto it = qubit_mapping.find(ref.name);
                if (it == qubit_mapping.end()) {
                    throw std::runtime_error("Unknown quantum register: " + ref.name);
                }
                
                if (ref.isRefWholeRegister()) {
                    // 展开整个寄存器
                    // 需要知道寄存器大小，这里简化处理
                    throw std::runtime_error("Whole register expansion not implemented");
                } else {
                    result.push_back(it->second + ref.index);
                }
            }
            return result;
        }
        
        // 展开经典寄存器引用
        std::vector<int> expand_register_refs_classical(const std::vector<RegisterRef>& refs) {
            std::vector<int> result;
            for (const auto& ref : refs) {
                auto it = clbit_mapping.find(ref.name);
                if (it == clbit_mapping.end()) {
                    throw std::runtime_error("Unknown classical register: " + ref.name);
                }
                
                if (ref.isRefWholeRegister()) {
                    throw std::runtime_error("Whole register expansion not implemented");
                } else {
                    result.push_back(it->second + ref.index);
                }
            }
            return result;
        }
        
        // 将AST表达式转换为Parameter
        Parameter expression_to_parameter(const Expression& expr) {
            ExpressionConverter converter;
            return converter.convert(expr);
        }
    };

};
};