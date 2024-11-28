#include "coupling.h"
#include <boost/graph/graph_utility.hpp>

CouplingCircuit::CouplingCircuit(CouplingList c_list) : c_list(c_list) {
    /* Convert a c_list to graph */
    // Add node
    std::set<int> qubits_set = {};
    std::vector<int> qubits_vec = {};        
    for (const auto& link : c_list) {
        qubits_set.insert(std::get<0>(link));
        qubits_set.insert(std::get<1>(link));
    }
    std::copy(qubits_set.begin(), qubits_set.end(), std::back_inserter(qubits_vec));
    for (int num : qubits_vec) {
        CouplingNode c_node{num};
        boost::add_vertex(c_node, graph);
    }
    // Update num qubits
    this->num_qubits = qubits_vec.size();
    // Add edge
    for (const auto& link : c_list) {
        CouplingEdge c_egde{std::get<2>(link)};
        boost::add_edge(std::get<0>(link), std::get<1>(link), c_egde, graph);
    } 
}