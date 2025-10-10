#pragma once
#include <cmath>
#include "AST/expression.hpp"
#include "AST/visitor.hpp"
#include "QuantumCircuit/include/circuit/parameter.h"

namespace qsteedcpp {
namespace qarser {

// 将AST Expression转换为Parameter
class ExpressionConverter : public AstVisitor {
private:
    Parameter result;
    
public:
    Parameter convert(const Expression& expr) {
        const_cast<Expression&>(expr).accept(*this);
        return result;
    }
    
    // 访问器实现
    void visit(Number& node) override {
        result = Parameter(node.value);
    }
    
    void visit(Pi& node) override {
        result = Parameter(M_PI);
    }
    
    void visit(Identifier& node) override {
        // 标识符作为参数变量
        result = Parameter(node.name);
    }
    
    void visit(BinaryOp& node) override {
        ExpressionConverter left_converter, right_converter;
        auto left_param = left_converter.convert(*node.left);
        auto right_param = right_converter.convert(*node.right);
        
        switch (node.op) {
            case BinaryOp::Op::ADD:
                result = left_param + right_param;
                break;
            case BinaryOp::Op::SUB:
                result = left_param - right_param;
                break;
            case BinaryOp::Op::MUL:
                result = left_param * right_param;
                break;
            case BinaryOp::Op::DIV:
                result = left_param / right_param;
                break;
            case BinaryOp::Op::POW:
                // TODO
                if (left_param.is_constant() && right_param.is_constant()) {
                    result = Parameter(std::pow(left_param.value(), right_param.value()));
                } else {
                    throw std::runtime_error("Power operation not supported for symbolic parameters");
                }
                break;
        }
    }
    
    void visit(UnaryOp& node) override {
        // 递归转换操作数
        ExpressionConverter operand_converter;
        auto operand_param = operand_converter.convert(*node.operand);
        
        switch (node.op) {
            case UnaryOp::Op::NEG:
                result = -operand_param;
                break;
            case UnaryOp::Op::SIN:
            case UnaryOp::Op::COS:
            case UnaryOp::Op::TAN:
            case UnaryOp::Op::EXP:
            case UnaryOp::Op::LN:
            case UnaryOp::Op::SQRT:
                // 这些函数可能需要Parameter类的扩展支持
                if (operand_param.is_constant()) {
                    double val = operand_param.value();
                    switch (node.op) {
                        case UnaryOp::Op::SIN:
                            result = Parameter(std::sin(val));
                            break;
                        case UnaryOp::Op::COS:
                            result = Parameter(std::cos(val));
                            break;
                        case UnaryOp::Op::TAN:
                            result = Parameter(std::tan(val));
                            break;
                        case UnaryOp::Op::EXP:
                            result = Parameter(std::exp(val));
                            break;
                        case UnaryOp::Op::LN:
                            result = Parameter(std::log(val));
                            break;
                        case UnaryOp::Op::SQRT:
                            result = Parameter(std::sqrt(val));
                            break;
                        default:
                            break;
                    }
                } else {
                    throw std::runtime_error("Transcendental functions not supported for symbolic parameters");
                }
                break;
        }
    }
    
    // 其他访问器（不需要实现）
    void visit(Program& node) override {}
    void visit(Include& node) override {}
    void visit(QRegister& node) override {}
    void visit(CRegister& node) override {}
    void visit(Gate& node) override {}
    void visit(Measure& node) override {}
    void visit(Barrier& node) override {}
    void visit(Reset& node) override {}
    void visit(GateDef& node) override {}
};

} // namespace qarser
} // namespace qsteedcpp