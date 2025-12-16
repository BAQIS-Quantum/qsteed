#include "torch_backend.hpp"
#include "gates_differentiable.hpp"
#include "tensor_network/index.hpp"
#include "circuit/circuit_instruction.h"
#include "expression/nodes/parameter.h"
#include "expression/nodes/constant.h"
#include "expression/nodes/binary_op.h"
#include "expression/nodes/unary_op.h"
#include <torch/torch.h>
#include <stdexcept>
#include <variant>
#include <algorithm>

namespace qsteedcpp {

// Helper to recursively find all Parameter nodes in an expression tree
void collect_param_nodes(const Expression* node, std::unordered_map<std::string, const Parameter*>& param_nodes) {
    if (!node) return;

    if (node->get_type() == Expression::Type::PARAMETER) {
        const auto* param_node = static_cast<const Parameter*>(node);
        // Only insert if not already present
        if (param_nodes.find(param_node->get_uuid()) == param_nodes.end()) {
            param_nodes[param_node->get_uuid()] = param_node;
        }
        return;
    }

    if (node->get_type() == Expression::Type::BINARY_OP) {
        const auto* binary_node = static_cast<const BinaryOp*>(node);
        collect_param_nodes(binary_node->get_left(), param_nodes);
        collect_param_nodes(binary_node->get_right(), param_nodes);
        return;
    }

    if (node->get_type() == Expression::Type::UNARY_OP) {
        const auto* unary_node = static_cast<const UnaryOp*>(node);
        collect_param_nodes(unary_node->get_operand(), param_nodes);
        return;
    }
}

// TorchExprEvaluator: 将Expression树转换为torch操作（保持梯度流）
class TorchExprEvaluator {
private:
    const std::unordered_map<std::string, torch::Tensor>& param_map_;

public:
    explicit TorchExprEvaluator(const std::unordered_map<std::string, torch::Tensor>& param_map)
        : param_map_(param_map) {}

    torch::Tensor eval(const Expr& expr) {
        const Expression* node = expr.get();
        return eval_node(node);
    }

private:
    torch::Tensor eval_node(const Expression* node) {
        switch (node->get_type()) {
            case Expression::Type::CONSTANT: {
                const auto* const_node = static_cast<const Constant*>(node);
                return torch::tensor(const_node->get_value(), torch::kDouble);
            }

            case Expression::Type::PARAMETER: {
                const auto* param_node = static_cast<const Parameter*>(node);
                std::string uuid = param_node->get_uuid();
                auto it = param_map_.find(uuid);
                if (it != param_map_.end()) {
                    return it->second;
                } else {
                    // Fallback: 如果找不到，使用默认值
                    return torch::tensor(param_node->get_value(), torch::kDouble);
                }
            }

            case Expression::Type::BINARY_OP: {
                const auto* binary_node = static_cast<const BinaryOp*>(node);
                torch::Tensor left = eval_node(binary_node->get_left());
                torch::Tensor right = eval_node(binary_node->get_right());

                switch (binary_node->get_op()) {
                    case BinaryOpType::ADD: return left + right;
                    case BinaryOpType::SUB: return left - right;
                    case BinaryOpType::MUL: return left * right;
                    case BinaryOpType::DIV: return left / right;
                    case BinaryOpType::POW: return torch::pow(left, right);
                }
                throw std::runtime_error("Unknown binary operator");
            }

            case Expression::Type::UNARY_OP: {
                const auto* unary_node = static_cast<const UnaryOp*>(node);
                torch::Tensor operand = eval_node(unary_node->get_operand());

                switch (unary_node->get_op()) {
                    case UnaryOpType::NEG:  return -operand;
                    case UnaryOpType::SIN:  return torch::sin(operand);
                    case UnaryOpType::COS:  return torch::cos(operand);
                    case UnaryOpType::TAN:  return torch::tan(operand);
                    case UnaryOpType::EXP:  return torch::exp(operand);
                    case UnaryOpType::LOG:  return torch::log(operand);
                    case UnaryOpType::SQRT: return torch::sqrt(operand);
                }
                throw std::runtime_error("Unknown unary operator");
            }
        }
        throw std::runtime_error("Unknown expression type");
    }
};

// 辅助函数：从表达式树中提取所有 Parameter 节点
static void extract_parameters_from_expr(
    const Expr& expr,
    std::unordered_map<std::string, double>& uuid_to_value) {

    auto uuids = expr.get_parameter_uuids();
    for (const auto& uuid : uuids) {
        if (uuid_to_value.find(uuid) == uuid_to_value.end()) {
            // 通过 eval() 获取当前值作为默认值
            uuid_to_value[uuid] = expr.eval();
        }
    }
}

double TorchBackend::get_param_default_value(const std::string& uuid) const {
    // 遍历所有gate，找到包含这个UUID的参数
    const auto& instructions = qc_.get_instructions();

    for (const auto& inst : instructions) {
        if (std::holds_alternative<std::unique_ptr<Gate>>(inst.operation)) {
            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(inst.operation);
            const Gate* gate = gate_ptr.get();

            if (gate->has_parameters()) {
                for (size_t i = 0; i < gate->parameter_count(); ++i) {
                    const auto& param_expr = gate->get_parameter_expression(i);
                    auto expr_uuids = param_expr.get_parameter_uuids();

                    if (expr_uuids.find(uuid) != expr_uuids.end()) {
                        // 找到了，返回表达式的求值结果作为默认值
                        return param_expr.eval();
                    }
                }
            }
        }
    }

    // 如果找不到，返回0（理论上不应该发生）
    return 0.0;
}

void TorchBackend::compile(const QuantumCircuit& qc) {
    std::cout << "[DEBUG] TorchBackend::compile() called" << std::endl;
    if (qc.num_qubits() == 0) {
        throw std::invalid_argument("Cannot compile empty quantum circuit");
    }

    qc_ = qc;
    std::cout << "[DEBUG] Circuit has " << qc.num_qubits() << " qubits" << std::endl;

    // 1. Collect all unique Parameter nodes from the circuit
    std::unordered_map<std::string, const Parameter*> param_nodes;
    const auto& instructions = qc_.get_instructions();
    for (const auto& inst : instructions) {
        if (std::holds_alternative<std::unique_ptr<Gate>>(inst.operation)) {
            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(inst.operation);
            if (gate_ptr->has_parameters()) {
                for (size_t i = 0; i < gate_ptr->parameter_count(); ++i) {
                    collect_param_nodes(gate_ptr->get_parameter_expression(i).get(), param_nodes);
                }
            }
        }
    }
    std::cout << "[DEBUG] Found " << param_nodes.size() << " unique parameter nodes" << std::endl;

    // 2. Create an ordered list of UUIDs
    param_uuids_.clear();
    for(const auto& pair : param_nodes) {
        param_uuids_.push_back(pair.first);
    }
    // Sort to ensure consistent order, which is important for reproducibility
    std::sort(param_uuids_.begin(), param_uuids_.end());

    // 3. Create torch::Tensors with correct requires_grad
    parameters_.clear();
    for (const auto& uuid : param_uuids_) {
        const Parameter* node = param_nodes.at(uuid);
        double value = node->get_value();
        bool trainable = node->is_trainable();

        std::cout << "[DEBUG] Creating tensor for param UUID: " << uuid.substr(0, 8) 
                  << "..., value: " << value << ", trainable: " << trainable << std::endl;
        
        auto opts = torch::TensorOptions().dtype(torch::kDouble).requires_grad(trainable);
        auto param_tensor = torch::tensor(value, opts);
        parameters_.push_back(param_tensor);
    }

    // 预编译所有gate的参数表达式
    std::cout << "[DEBUG] Precompiling gate parameter expressions" << std::endl;
    gate_param_evaluators_.clear();
    const auto& insts = qc_.get_instructions();

    for (size_t inst_idx = 0; inst_idx < insts.size(); ++inst_idx) {
        const auto& inst = insts[inst_idx];
        std::vector<std::function<torch::Tensor(const std::unordered_map<std::string, torch::Tensor>&)>> gate_evals;

        if (std::holds_alternative<std::unique_ptr<Gate>>(inst.operation)) {
            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(inst.operation);
            const Gate* gate = gate_ptr.get();
            std::cout << "[DEBUG] Instruction " << inst_idx << ": " << gate->name();

            if (gate->has_parameters()) {
                std::cout << " with " << gate->parameter_count() << " parameters" << std::endl;
                for (size_t i = 0; i < gate->parameter_count(); ++i) {
                    // 捕获表达式的副本
                    Expr expr = gate->get_parameter_expression(i);
                    std::cout << "[DEBUG]   Parameter " << i << ": " << expr.to_string() << std::endl;

                    // 预编译成lambda函数
                    gate_evals.push_back(
                        [expr](const std::unordered_map<std::string, torch::Tensor>& param_map) -> torch::Tensor {
                            TorchExprEvaluator evaluator(param_map);
                            return evaluator.eval(expr);
                        }
                    );
                }
            } else {
                std::cout << " (no parameters)" << std::endl;
            }
        }

        gate_param_evaluators_.push_back(std::move(gate_evals));
    }

    std::cout << "[DEBUG] Compilation complete" << std::endl;
    compiled_ = true;
}

void TorchBackend::apply_single_qubit_gate(
    TensorNetwork& tn,
    GateType gate_type,
    const std::vector<torch::Tensor>& params,
    int target,
    size_t gate_idx,
    std::vector<Index>& current_indices) {

    Index in_idx = current_indices[target];
    Index out_idx(2, "q" + std::to_string(target) + "_g" + std::to_string(gate_idx));

    torch::Tensor gate_matrix = make_gate_matrix_differentiable(gate_type, params);
    ITensor gate_tensor(gate_matrix, {in_idx, out_idx});
    tn.add_tensor(gate_tensor);

    current_indices[target] = out_idx;
}

void TorchBackend::apply_two_qubit_gate(
    TensorNetwork& tn,
    GateType gate_type,
    const std::vector<torch::Tensor>& params,
    int control,
    int target,
    size_t gate_idx,
    std::vector<Index>& current_indices) {

    Index control_in = current_indices[control];
    Index target_in = current_indices[target];
    Index control_out(2, "q" + std::to_string(control) + "_g" + std::to_string(gate_idx));
    Index target_out(2, "q" + std::to_string(target) + "_g" + std::to_string(gate_idx));

    torch::Tensor gate_matrix = make_gate_matrix_differentiable(gate_type, params);
    gate_matrix = gate_matrix.reshape({2, 2, 2, 2});

    ITensor gate_tensor(gate_matrix, {control_in, target_in, control_out, target_out});
    tn.add_tensor(gate_tensor);

    current_indices[control] = control_out;
    current_indices[target] = target_out;
}

torch::Tensor TorchBackend::run_internal() {
    std::cout << "[DEBUG] run_internal() started" << std::endl;

    // 构建内部 param_map
    std::unordered_map<std::string, torch::Tensor> param_map;
    for (size_t i = 0; i < param_uuids_.size(); ++i) {
        param_map[param_uuids_[i]] = parameters_[i];
    }
    std::cout << "[DEBUG] Built param_map with " << param_map.size() << " parameters" << std::endl;

    // 构建 tensor network
    TensorNetwork tn;
    const auto& instructions = qc_.get_instructions();
    std::cout << "[DEBUG] Processing " << instructions.size() << " instructions" << std::endl;

    std::vector<Index> current_indices;
    std::vector<Index> input_indices;

    // Create initial state indices
    int num_qubits = qc_.num_qubits();
    for (int q = 0; q < num_qubits; ++q) {
        Index in_idx(2, "q" + std::to_string(q) + "_init");
        current_indices.push_back(in_idx);
        input_indices.push_back(in_idx);
    }

    // Process each instruction
    for (size_t gate_idx = 0; gate_idx < instructions.size(); ++gate_idx) {
        std::cout << "[DEBUG] Processing gate " << gate_idx << std::endl;
        const auto& inst = instructions[gate_idx];

        if (std::holds_alternative<std::unique_ptr<Gate>>(inst.operation)) {
            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(inst.operation);
            const Gate* gate = gate_ptr.get();
            GateType gate_type = gate->type();
            std::cout << "[DEBUG] Gate type: " << gate->name() << std::endl;

            // 使用预编译的参数求值函数（性能优化）
            std::vector<torch::Tensor> params;
            std::cout << "[DEBUG] Evaluating " << gate_param_evaluators_[gate_idx].size() << " parameters" << std::endl;
            for (const auto& param_evaluator : gate_param_evaluators_[gate_idx]) {
                std::cout << "[DEBUG] Before param_evaluator call" << std::endl;
                params.push_back(param_evaluator(param_map));
                std::cout << "[DEBUG] After param_evaluator call" << std::endl;
            }

            // Dispatch based on gate type
            switch (gate_type) {
                case GateType::H:
                case GateType::X:
                case GateType::Y:
                case GateType::Z:
                case GateType::S:
                case GateType::SDG:
                case GateType::T:
                case GateType::TDG:
                case GateType::RX:
                case GateType::RY:
                case GateType::RZ:
                case GateType::P:
                    apply_single_qubit_gate(tn, gate_type, params, inst.qubits[0], gate_idx, current_indices);
                    break;

                case GateType::CNOT:
                case GateType::CZ:
                case GateType::SWAP:
                case GateType::ISWAP:
                case GateType::RXX:
                case GateType::RYY:
                case GateType::RZZ:
                case GateType::CP:
                    apply_two_qubit_gate(tn, gate_type, params, inst.qubits[0], inst.qubits[1], gate_idx, current_indices);
                    break;

                default:
                    throw std::runtime_error("Unsupported gate type: " + std::string(gate->name()));
            }
        }
    }

    // Add initial states |0⟩
    std::cout << "[DEBUG] Adding initial states for " << num_qubits << " qubits" << std::endl;
    for (int q = 0; q < num_qubits; ++q) {
        // Create |0⟩ state: [1+0i, 0+0i] as a proper complex tensor
        using c = c10::complex<double>;
        torch::Tensor zero_state = torch::stack({
            torch::tensor(c(1.0, 0.0), torch::kComplexDouble),
            torch::tensor(c(0.0, 0.0), torch::kComplexDouble)
        });
        ITensor state_tensor(zero_state, {input_indices[q]});
        tn.add_tensor(state_tensor);
    }
    std::cout << "[DEBUG] Initial states added" << std::endl;

    // Contract and return
    std::cout << "[DEBUG] Starting tensor network contraction" << std::endl;
    ITensor result = tn.contract_all();
    std::cout << "[DEBUG] Contraction completed" << std::endl;

    // IMPORTANT: Copy tensor to avoid dangling reference when result goes out of scope
    // torch::Tensor copy constructor is shallow (shares data), preserves gradient graph
    torch::Tensor result_tensor = result.tensor();
    std::cout << "[DEBUG] Returning result tensor" << std::endl;
    return result_tensor;
}

torch::Tensor TorchBackend::run() {
    std::cout << "[DEBUG] TorchBackend::run() called" << std::endl;
    if (!compiled_) {
        throw std::runtime_error("Circuit not compiled. Call compile() first.");
    }

    std::cout << "[DEBUG] Calling run_internal()" << std::endl;
    torch::Tensor result = run_internal();
    std::cout << "[DEBUG] run_internal() completed successfully" << std::endl;
    std::cout << "[DEBUG] Result tensor shape: " << result.sizes() << std::endl;
    std::cout << "[DEBUG] Result tensor dtype: " << result.dtype() << std::endl;
    std::cout << "[DEBUG] About to return from run()" << std::endl;

    // Force deep copy to test if it's a memory issue
    torch::Tensor result_copy = result.clone();
    std::cout << "[DEBUG] Cloned result tensor" << std::endl;
    return result_copy;
}

torch::Tensor TorchBackend::get_parameter(const Parameter& param) const {
    if (!compiled_) {
        throw std::runtime_error("Circuit not compiled. Call compile() first.");
    }

    std::string uuid = param.get_uuid();

    // 在 param_uuids_ 中查找对应的索引
    auto it = std::find(param_uuids_.begin(), param_uuids_.end(), uuid);

    if (it != param_uuids_.end()) {
        size_t index = std::distance(param_uuids_.begin(), it);
        return parameters_[index];
    }

    throw std::runtime_error("Parameter not found in compiled circuit. "
                           "Make sure the parameter is used in the circuit.");
}

}  // namespace qsteedcpp
