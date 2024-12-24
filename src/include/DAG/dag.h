#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include "sabre_core.h"
#include "instructionNode.h"
#include "edge.h"

#include "vendor/prettyprint.hpp"

using namespace sabre;

using DagGraph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, InstructionNode, EdgeProperties>;


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

    void add_instruction_node_end(const InstructionNode& node) {
        if (!this->empty()) {
            const node_pos_t node_index = add_node(node);
            std::vector<DagGraph::edge_descriptor> edges_to_remove;
            std::vector<Edge> edges_to_add;
            std::vector<qubit_t> node_remain_qubits = node.qubit_pos;  
              
            DagGraph::in_edge_iterator ei, ei_end;
            for (boost::tie(ei, ei_end) = boost::in_edges(end_node_pos, graph); ei != ei_end; ++ei) {
                const qubit_t qubit = graph[*ei].qubit_id; 
                // 判断这条边的qubit是否在node的qubit_pos中
                if ( std::find(node.qubit_pos.begin(), node.qubit_pos.end(), qubit) != node.qubit_pos.end() ) {
                    edges_to_remove.push_back(*ei);
                    edges_to_add.emplace_back(boost::source(*ei, graph), node_index, qubit);
                    edges_to_add.emplace_back(node_index, end_node_pos, qubit);
                    node_remain_qubits.erase(std::remove(node_remain_qubits.begin(), node_remain_qubits.end(), qubit), node_remain_qubits.end());     
                }
            }
            for (const auto& edge: edges_to_remove) {
                boost::remove_edge(edge, graph);
            }
            for (const auto& edge: edges_to_add) {
                boost::add_edge(edge.source, edge.target, edge.ep, graph);
            }
            for (const auto& qubit: node_remain_qubits) {
                add_edge(start_node_pos, node_index, qubit);
                add_edge(node_index, end_node_pos, qubit);
            }

        } else {
            add_node(InstructionNode("start"));
            add_node(InstructionNode("end"));
            add_node(node);
            for (const auto& qubit: node.qubit_pos) {
                add_edge(start_node_pos, 2, EdgeProperties{qubit});
                add_edge(2, end_node_pos, EdgeProperties{qubit});
            }
        }

        // std::cout << this->get_qubits_used() << std::endl;
    }

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

#ifdef WITH_GRAPHVIZ
    void draw() const;
#endif


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

DagGraph reverse_DagGraph(const DagGraph& graph);