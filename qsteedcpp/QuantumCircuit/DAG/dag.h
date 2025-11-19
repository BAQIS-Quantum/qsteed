#pragma once
#include <unordered_map>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include "instructionNode.h"
#include "edge.h"

// #include "vendor/prettyprint.hpp"

using DagGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, InstructionNode, EdgeProperties>;
void draw_graph(const DagGraph& graph);

class DAGCircuit
/*
   A directed acyclic graph (DAG) representation of a quantum circuit.

   Using boost graph library to represent the DAG.
   Each vertex is an InstructionNode which represents a quantum gate or operation.
   Each edge is an EdgeProperties which represents qubit.
*/
{
public:
    DagGraph graph;
    std::unordered_map<qubit_t, cbit_t> measure = {};

    node_pos_t start_node_pos = 0;
    node_pos_t end_node_pos = 1;

public:
    DAGCircuit() {}
    DAGCircuit(DagGraph& graph) : graph(graph) {}

    int add_node(const InstructionNode& node) {
        auto vertex_description = boost::add_vertex(node, graph);
        return vertex_description;
    }

    void add_edge(const int from, const int to, const EdgeProperties& ep) {
        boost::add_edge(from, to, ep, graph); 
    }
    void add_edge(const int from, const int to, const qubit_t qubit_id) {
        boost::add_edge(from, to, EdgeProperties{qubit_id}, graph);
    }

    int get_num_nodes() const {
        return boost::num_vertices(graph); 
    }

    bool empty() const {
        return get_num_nodes() == 0;
    }


    std::set<int> get_qubits_used() const {
        std::set<int> qubits_id_set;      
        DagGraph::edge_iterator ei, ei_end;
        for (boost::tie(ei, ei_end) = boost::edges(graph); ei != ei_end; ++ei)
            qubits_id_set.insert(graph[*ei].qubit_id);
        return qubits_id_set;
    }

    int num_qubits() const {
        return get_qubits_used().size();
    }

    void add_instruction_node_end(const InstructionNode& node);

    DAGCircuit reverse() const {

        DagGraph rev_graph;
        boost::copy_graph(boost::make_reverse_graph(this->graph), rev_graph);
        DAGCircuit rev_dag {rev_graph};

        rev_dag.start_node_pos = this->end_node_pos;
        rev_dag.end_node_pos = this->start_node_pos;
        rev_dag.graph[rev_dag.start_node_pos].name = "start";
        rev_dag.graph[rev_dag.end_node_pos].name = "end";

        return rev_dag;
    }



    DagGraph::vertex_iterator vertex_begin() const{
        return boost::vertices(graph).first;
    }
    DagGraph::vertex_iterator vertex_end() const {
        return boost::vertices(graph).second;
    }

// #ifdef WITH_GRAPHVIZ
//     void draw() const {
//         draw_graph(this->graph);
//     }
// #endif


private:
    bool _is_start_exist() const {
        return get_num_nodes() != 0 && graph[start_node_pos].name == "start"; 
    }

    bool _is_end_exist() const {
        return get_num_nodes() != 0 && graph[end_node_pos].name == "end"; 
    }

    void remove_edge(node_pos_t source, node_pos_t target) {
        boost::remove_edge(source, target, graph);
    }
    void remove_edge(Edge edge) {
        boost::remove_edge(edge.source, edge.target, graph);
    }

};

inline DagGraph reverse_DagGraph(const DagGraph& graph) {
    DagGraph rev_graph;
    boost::copy_graph(boost::make_reverse_graph(graph), rev_graph);
    return rev_graph;
}