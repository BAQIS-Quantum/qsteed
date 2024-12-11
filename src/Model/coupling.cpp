#include "coupling.h"
#include <cmath>
#include <stdexcept>


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



Matrix CouplingCircuit::get_distance_matrix() const {
    Matrix distance_matrix(boost::num_vertices(graph), std::vector<int>(boost::num_vertices(graph)));
    boost::constant_property_map<CouplingGraph::edge_descriptor, int> weight_map(1);

    bool success = boost::floyd_warshall_all_pairs_shortest_paths(graph, distance_matrix, boost::weight_map(weight_map));
    return distance_matrix;
}



std::map<std::pair<int, int>, double> CouplingCircuit::get_fidelity_dict() const {
    std::map<std::pair<int, int>, double> fidelity_dict;

    auto vertices = boost::vertices(graph);      
    for (auto i=vertices.first; i!=vertices.second; ++i) {
        for (auto j=vertices.first; j!=vertices.second; ++j) {
            if (*i == *j)
                continue;
            fidelity_dict[std::make_pair(*i, *j)] =  this->get_fidelity(*i, *j);
        }
    }
    return fidelity_dict;
};


/**
 * @brief Calculate the fidelity of the path from source to target.
 * 
 * This function assumes that there are paths from source to target 
 * and from target to source. If the paths do not exist, the function 
 * will throw an exception.
 * 
 * @param source The source node.
 * @param target The target node.
 * @return double The fidelity of the path.
 */
double CouplingCircuit::get_fidelity(int source, int target) const {
    double fidelity = 0;
    CGEdgeDesc edge;
    bool exists;

    std::vector<int> path = this->get_mini_path(source, target);
    if (path.size() == 0) {
        return fidelity;
    } 
    if (path.size() == 2) {
        std::tie(edge, exists) = boost::edge(path[0], path[1], graph);
        fidelity = std::log(graph[edge].fidelity);
    } 
    else {
        for (size_t i = 0; i < path.size() - 2; ++i) { // SWAP gates need to be inserted
            double min_f = std::min(std::log(graph[boost::edge(path[i], path[i + 1], graph).first].fidelity),
                                    std::log(graph[boost::edge(path[i + 1], path[i], graph).first].fidelity));
            double max_f = std::max(std::log(graph[boost::edge(path[i], path[i + 1], graph).first].fidelity),
                                    std::log(graph[boost::edge(path[i + 1], path[i], graph).first].fidelity));
            fidelity += 2 * max_f + min_f;
        }
        fidelity += std::log(graph[boost::edge(path.back(), path[path.size() - 2], graph).first].fidelity);
    }

    return fidelity;
}


std::vector<int> CouplingCircuit::get_mini_path(int source, int target) const {
    std::vector<int> path;
    std::vector<int> predecessors(boost::num_vertices(graph));
    std::vector<double> distances(boost::num_vertices(graph));

    // BFS algorithm do better ?
    // boost::breadth_first_search(
    //     graph, 
    //     source,
    //     boost::visitor(boost::make_bfs_visitor(boost::record_predecessors(&predecessors[0], boost::on_tree_edge())))
    // );

    boost::dijkstra_shortest_paths(
        graph, 
        source,
        boost::predecessor_map(&predecessors[0])
            .distance_map(&distances[0])
            .weight_map(boost::make_constant_property<CGEdgeDesc>(1.0))
    );

    // No path from source to target    
    if (target == predecessors[target]) {
        return path;
    }

    for (int v = target; v != source; v = predecessors[v]) {
        path.push_back(v);
    }
    path.push_back(source);
    std::reverse(path.begin(), path.end());

    return path;
}