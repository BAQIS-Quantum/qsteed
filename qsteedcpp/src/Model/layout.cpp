#include "layout.h"
#include <numeric>

Layout generate_random_layout(int virtual_qubits, int physical_qubits) {
    if (virtual_qubits > physical_qubits) 
        throw std::invalid_argument("Error: The number of virtual qubits in the circuit cannot be greater than the number of physical qubits in the chip.");

    // Generate a ordered sequence of virtual qubits
    std::vector<int> virtual_qubit(virtual_qubits);
    std::iota(virtual_qubit.begin(), virtual_qubit.end(), 0);

    // Generate a random permutation of the physical qubits
    std::vector<int> physical_qubit(physical_qubits);
    std::iota(physical_qubit.begin(), physical_qubit.end(), 0);
    std::mt19937 rng(std::random_device{}());
    std::shuffle(physical_qubit.begin(), physical_qubit.end(), rng);

    // 创建一个从虚拟量子比特到物理量子比特的映射
    std::unordered_map<int, int> v2p;
    for (int i = 0; i < virtual_qubits; ++i)
        v2p[virtual_qubit[i]] = physical_qubit[i];

    return Layout{v2p};
}



void print_layout(const LayoutStructure& layout) {
    std::cout << "{";
    for (auto it = layout.begin(); it != layout.end(); ++it) {
        std::cout << it->first << ": " << it->second;
        if (std::next(it) != layout.end()) {
            std::cout << ", ";
        }
    }
    std::cout << "}" << std::endl;
}
