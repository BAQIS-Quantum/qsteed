#include <torch/torch.h>
#include <ATen/ops/tensordot.h>

#include <unordered_map>
#include <unordered_set>

#include "itensor.hpp"


namespace qsteedcpp {

// Implementation of simple tensordot-based contraction for two ITensors.
ITensor operator*(const ITensor& A, const ITensor& B) {
	std::vector<long> dims_A;
	std::vector<long> dims_B;

	std::vector<Index> uncontracted_indices_A;
	std::vector<Index> uncontracted_indices_B;

	std::unordered_map<long, long> b_index_id_to_axis;
	for (long i = 0; i < B.indices().size(); ++i) {
		b_index_id_to_axis[B.indices()[i].id()] = i;
	}

	std::unordered_set<long> shared_index_ids; // Moved here and populated in this loop
	for (long i = 0; i < A.indices().size(); ++i) {
		const auto& idx_A = A.indices()[i];
		auto it = b_index_id_to_axis.find(idx_A.id());
		if (it != b_index_id_to_axis.end()) {
			dims_A.push_back(i);
			dims_B.push_back(it->second);
			shared_index_ids.insert(idx_A.id()); // Populate shared_index_ids here
		} else {
			uncontracted_indices_A.push_back(idx_A);
		}
	}

	for (const auto& idx_B : B.indices()) {
		if (shared_index_ids.find(idx_B.id()) == shared_index_ids.end()) {
			uncontracted_indices_B.push_back(idx_B);
		}
	}

	// Sanity check: dims_A and dims_B must have the same size
	if (dims_A.size() != dims_B.size()) {
		throw std::runtime_error("Internal error: Mismatch in sizes of contraction dimensions for tensordot.");
	}

	// Perform tensordot
	torch::Tensor result_tensor = torch::tensordot(A.tensor(), B.tensor(), dims_A, dims_B);

	// Reconstruct result_indices_vec
	// tensordot output order: uncontracted axes of self, then uncontracted axes of other
	std::vector<Index> result_indices_vec;
	result_indices_vec.reserve(uncontracted_indices_A.size() + uncontracted_indices_B.size());
	result_indices_vec.insert(result_indices_vec.end(), uncontracted_indices_A.begin(), uncontracted_indices_A.end());
	result_indices_vec.insert(result_indices_vec.end(), uncontracted_indices_B.begin(), uncontracted_indices_B.end());

	return ITensor(result_tensor, result_indices_vec);
}



};

