#pragma once
#include "tensor_network/tensor_network.hpp"
#include "circuit/quantum_circuit.h"
#include <torch/torch.h>
#include <vector>
#include <unordered_map>
#include <string>

namespace qsteedcpp {

/**
 * TorchBackend - 支持自动微分的量子线路后端
 *
 * 特点：
 * - 自动管理参数：compile 时自动为线路中的参数创建 torch::Tensor
 * - 极简接口：run() 不需要手动传参数
 * - 支持 autograd：所有参数默认 requires_grad=true
 *
 * 用法：
 *   TorchBackend backend;
 *   backend.compile(qc);
 *   auto result = backend.run();
 *   loss.backward();
 *   // 通过 backend.parameters() 访问参数
 */
class TorchBackend {
private:
    QuantumCircuit qc_;
    bool compiled_ = false;

    // 内部管理的参数
    std::vector<torch::Tensor> parameters_;
    std::vector<std::string> param_uuids_;  // 对应的UUID

    // 预编译的参数求值函数 (优化性能)
    // gate_param_evaluators_[i][j] = 第i个gate的第j个参数的求值函数
    std::vector<std::vector<std::function<torch::Tensor(const std::unordered_map<std::string, torch::Tensor>&)>>> gate_param_evaluators_;

    // 辅助方法：根据UUID从qc获取默认值
    double get_param_default_value(const std::string& uuid) const;

    // 内部方法：使用当前参数构建并运行tensor network
    torch::Tensor run_internal();

    // Helper methods
    void apply_single_qubit_gate(
        TensorNetwork& tn,
        GateType gate_type,
        const std::vector<torch::Tensor>& params,
        int target,
        size_t gate_idx,
        std::vector<Index>& current_indices);

    void apply_two_qubit_gate(
        TensorNetwork& tn,
        GateType gate_type,
        const std::vector<torch::Tensor>& params,
        int control,
        int target,
        size_t gate_idx,
        std::vector<Index>& current_indices);

public:
    TorchBackend() : qc_(0) {}

    /**
     * 编译量子线路
     * - 保存线路
     * - 自动提取所有参数并创建对应的 torch::Tensor
     */
    void compile(const QuantumCircuit& qc);

    /**
     * 运行线路（使用内部管理的参数）
     * 返回的 Tensor 连接到参数的计算图，支持 backward()
     */
    torch::Tensor run();

    /**
     * 获取所有参数（可以传给 optimizer）
     * 注意：返回的 Tensor 与内部参数共享存储，修改会影响后续 run()
     */
    std::vector<torch::Tensor> parameters() const {
        return parameters_;
    }

    /**
     * 获取单个参数（通过索引）
     */
    torch::Tensor parameter(size_t index) const {
        if (index >= parameters_.size()) {
            throw std::out_of_range("Parameter index out of range");
        }
        return parameters_[index];
    }

    /**
     * 获取单个参数（通过 Parameter 对象）
     * 这是用户友好的接口，避免用户手动查找索引
     */
    torch::Tensor get_parameter(const Parameter& param) const;

    /**
     * 参数数量
     */
    size_t num_parameters() const {
        return parameters_.size();
    }

    /**
     * 设置参数值（可选，通常不需要手动调用）
     */
    void set_parameter(size_t index, const torch::Tensor& value) {
        if (index >= parameters_.size()) {
            throw std::out_of_range("Parameter index out of range");
        }
        parameters_[index] = value;
    }

    bool is_compiled() const { return compiled_; }

    int num_qubits() const { return qc_.num_qubits(); }

    void reset() {
        qc_ = QuantumCircuit(0);
        compiled_ = false;
        parameters_.clear();
        param_uuids_.clear();
        gate_param_evaluators_.clear();
    }
};

}  // namespace qsteedcpp
