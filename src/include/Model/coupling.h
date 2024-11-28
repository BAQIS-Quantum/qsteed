#pragma once
#include <map>
#include <vector>
#include <tuple>
#include <memory>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_utility.hpp>

using CouplingList = std::vector<std::tuple<int, int, float>>;
const CouplingList EMPTY_COUPLING_LIST = {std::make_tuple(0, 0, 0)};

using Matrix = std::vector<std::vector<int>>;

struct CouplingNode {
    int id;
};
struct CouplingEdge {
    double fidelity;
};

using CouplingGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, CouplingNode, CouplingEdge>;
using EdgeDesc = boost::graph_traits<CouplingGraph>::edge_descriptor;


class CouplingCircuit {
public:     
    CouplingGraph graph;
    CouplingList c_list;
    bool is_bidirectional = true;    
    unsigned int num_qubits = 0;


public:
    CouplingCircuit(CouplingList c_list);


    void update_num_qubits() { 
        this->num_qubits = boost::num_vertices(graph); 
    }

    Matrix get_distance_matrix() const {
        Matrix distance_matrix(boost::num_vertices(graph), std::vector<int>(boost::num_vertices(graph)));
        boost::constant_property_map<CouplingGraph::edge_descriptor, int> weight_map(1);

        bool success = boost::floyd_warshall_all_pairs_shortest_paths(graph, distance_matrix, boost::weight_map(weight_map));
        return distance_matrix;
    }

    std::map<std::pair<int, int>, double> get_fidelity_dict() const {
        std::map<std::pair<int, int>, double> path_fidelity;

        return path_fidelity;
    }

    double get_fidelity(int source, int target) const {
        double fidelity = 0.0;
         
        std::cout << boost::vertex(source, graph) << std::endl;

        return fidelity;
    } 


    std::vector<int> get_mini_path(int source, int target) const {
        std::vector<int> path;
        std::vector<int> predecessors(boost::num_vertices(graph));
        std::vector<double> distances(boost::num_vertices(graph));

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
                .weight_map(boost::make_constant_property<EdgeDesc>(1.0))
        );

        for (auto p : predecessors) {
            std::cout << p << ", ";
        }
        std::cout << std::endl;


        return path;
    }




    void print() const {
        print_graph(graph);
    }
};