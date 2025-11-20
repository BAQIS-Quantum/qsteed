#pragma once

#include <torch/torch.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <functional>

#include "index.hpp"

namespace qsteedcpp {

class ITensor {
private:
    mutable torch::Tensor data_;  // Mutable to allow recomputation in const tensor() method
    std::vector<Index> indices_;

    // For parametric gates: holds parameter reference and computation function
    torch::Tensor param_;  // Parameter reference (shallow copy, shares data)
    std::function<torch::Tensor(const torch::Tensor&)> gate_func_;  // Gate computation function

public:
    // Constructor for static gates (H, X, CNOT, etc.)
    ITensor(const torch::Tensor& data, const std::vector<Index>& indices)
        : data_(data), indices_(indices) {  // Don't clone - preserve gradient graph!

        // Validation: The number of indices must match the rank of the tensor.
        if (data.dim() != static_cast<int64_t>(indices.size())) {
            throw std::invalid_argument(
                "Number of indices (" + std::to_string(indices.size()) +
                ") must match tensor rank (" + std::to_string(data.dim()) + ")."
            );
        }

        // Validation: The dimension of each index must match the tensor's shape.
        for (size_t i = 0; i < indices.size(); ++i) {
            if (indices[i].dim() != data.size(static_cast<int64_t>(i))) {
                throw std::invalid_argument(
                    "Dimension of index " + std::to_string(i) + " (" + indices[i].name() + ", dim=" + std::to_string(indices[i].dim()) +
                    ") does not match tensor shape at that position (" + std::to_string(data.size(static_cast<int64_t>(i))) + ")."
                );
            }
        }
    }

    // Constructor for parametric gates (RX, RY, RZ, etc.)
    ITensor(const torch::Tensor& param,
            std::function<torch::Tensor(const torch::Tensor&)> gate_func,
            const std::vector<Index>& indices)
        : param_(param),  // Shallow copy - shares data with original tensor
          gate_func_(gate_func),
          indices_(indices) {

        // Compute initial gate matrix
        data_ = gate_func_(param_);

        // Validation
        if (data_.dim() != static_cast<int64_t>(indices.size())) {
            throw std::invalid_argument(
                "Number of indices must match tensor rank for parametric gate."
            );
        }
    }

    // Getters
    const torch::Tensor& tensor() const {
        // If this is a parametric gate, recompute with latest parameter value
        if (gate_func_) {
            data_ = gate_func_(param_);
        }
        return data_;
    }

    const std::vector<Index>& indices() const { return indices_; }

}; // class ITensor


ITensor operator*(const ITensor& A, const ITensor& B);



}; // namespace qsteedcpp
