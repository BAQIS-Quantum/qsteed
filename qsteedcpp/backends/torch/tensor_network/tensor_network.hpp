#pragma once
#include <ATen/ops/tensordot.h>
#include <torch/torch.h>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include "itensor.hpp"

namespace qsteedcpp {


class TensorNetwork {
public:
    struct Edge {
        size_t tensor_a;     // Index of first tensor
        size_t tensor_b;     // Index of second tensor
        long index_id;       // ID of the shared Index

        Edge(size_t a, size_t b, long id)
            : tensor_a(a), tensor_b(b), index_id(id) {}
    };

private:
    std::vector<ITensor> tensors_;
    std::vector<Edge> edges_;

    // Map: index_id -> list of tensor indices that have this index
    std::unordered_map<long, std::vector<size_t>> index_to_tensors_;

public:
    TensorNetwork() = default;

    void add_tensor(const ITensor& tensor) {
        size_t new_idx = tensors_.size();
        tensors_.push_back(tensor);

        auto_detect_edges(new_idx);
    }

    size_t size() const {
        return tensors_.size();
    }

    // Clear the entire network (tensors, edges, and index mappings)
    void clear() {
        tensors_.clear();
        edges_.clear();
        index_to_tensors_.clear();
    }

    const ITensor& get_tensor(size_t index) const {
        if (index >= tensors_.size()) {
            throw std::out_of_range("Tensor index out of bounds.");
        }
        return tensors_[index];
    }

    // Update an existing tensor at the given index
    // The new tensor must have the same indices as the old one to preserve graph structure
    void update_tensor(size_t index, const ITensor& new_tensor) {
        if (index >= tensors_.size()) {
            throw std::out_of_range("Tensor index out of bounds.");
        }

        // Note: We don't need to update edges because the indices should remain the same
        tensors_[index] = new_tensor;
    }

    // Get all edges in the network
    const std::vector<Edge>& get_edges() const {
        return edges_;
    }

    // Get tensors that are connected to the given tensor
    std::vector<size_t> get_neighbors(size_t tensor_idx) const {
        std::unordered_set<size_t> neighbors;

        for (const auto& edge : edges_) {
            if (edge.tensor_a == tensor_idx) {
                neighbors.insert(edge.tensor_b);
            } else if (edge.tensor_b == tensor_idx) {
                neighbors.insert(edge.tensor_a);
            }
        }

        return std::vector<size_t>(neighbors.begin(), neighbors.end());
    }

    // Get all dangling indices (indices that appear in only one tensor)
    std::vector<long> get_dangling_indices() const {
        std::unordered_map<long, size_t> index_count;

        // Count how many tensors each index appears in
        for (const auto& tensor : tensors_) {
            for (const auto& idx : tensor.indices()) {
                index_count[idx.id()]++;
            }
        }

        // Find indices that appear only once (dangling)
        std::vector<long> dangling;
        for (const auto& [idx_id, count] : index_count) {
            if (count == 1) {
                dangling.push_back(idx_id);
            }
        }

        return dangling;
    }

    // Print the network structure for debugging
    void print_structure() const {
        std::cout << "=== Tensor Network Structure ===" << std::endl;
        std::cout << "Tensors: " << tensors_.size() << std::endl;
        std::cout << "Edges: " << edges_.size() << std::endl;

        for (size_t i = 0; i < tensors_.size(); ++i) {
            std::cout << "  Tensor " << i << ": "
                      << tensors_[i].indices().size() << " indices" << std::endl;
        }

        std::cout << "\nEdges:" << std::endl;
        for (const auto& edge : edges_) {
            std::cout << "  Tensor " << edge.tensor_a
                      << " <--(Index " << edge.index_id << ")--> "
                      << "Tensor " << edge.tensor_b << std::endl;
        }

        auto dangling = get_dangling_indices();
        std::cout << "\nDangling indices: " << dangling.size() << std::endl;
    }

    ITensor contract_all() {
        if (tensors_.empty()) {
            throw std::runtime_error("Cannot contract an empty tensor network.");
        }
        if (tensors_.size() == 1) {
            return tensors_[0];
        }

        ITensor result = tensors_[0];
        for (size_t i = 1; i < tensors_.size(); ++i) {
            result = result * tensors_[i];
        }
        return result;
    }

private:
    // Automatically detect edges when a new tensor is added
    void auto_detect_edges(size_t new_tensor_idx) {
        const auto& new_tensor = tensors_[new_tensor_idx];
        const auto& new_indices = new_tensor.indices();

        for (const auto& new_idx : new_indices) {
            long idx_id = new_idx.id();

            auto it = index_to_tensors_.find(idx_id);
            if (it != index_to_tensors_.end()) {
                // This index is shared! Create edges with all tensors that have it
                for (size_t old_tensor_idx : it->second) {
                    edges_.emplace_back(old_tensor_idx, new_tensor_idx, idx_id);
                }

                it->second.push_back(new_tensor_idx);
            } else {
                // First time seeing this index, create a new entry
                index_to_tensors_[idx_id] = {new_tensor_idx};
            }
        }
    }
};


}; // namespace qsteedcpp