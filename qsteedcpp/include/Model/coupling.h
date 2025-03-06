#pragma once
#include <map>
#include <vector>
#include <tuple>
#include <memory>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>


using CouplingList = std::vector<std::tuple<int, int, float>>;
const CouplingList EMPTY_COUPLING_LIST = {std::make_tuple(0, 0, 0.0f)};

using Matrix = std::vector<std::vector<int>>;

struct CouplingNode {
    int id;
};
struct CouplingEdge {
    double fidelity;
};

using CouplingGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, CouplingNode, CouplingEdge>;
using CGEdgeDesc = boost::graph_traits<CouplingGraph>::edge_descriptor;


class CouplingCircuit {
public:     
    CouplingGraph graph;
    CouplingList c_list;
    bool is_bidirectional = true;
    unsigned int num_qubits = 0;


public:
    CouplingCircuit(CouplingList c_list);

    void print() const {
        boost::print_graph(graph);
    }

    void update_num_qubits() { 
        this->num_qubits = boost::num_vertices(graph); 
    }

    Matrix get_distance_matrix() const;

    std::map<std::pair<int, int>, double> get_fidelity_dict() const;

    double get_fidelity(int source, int target) const;

    std::vector<int> get_mini_path(int source, int target) const;

};