#include "dag.h"


void DAGCircuit::add_instruction_node_end(const InstructionNode& node) {
    if (!this->empty()) {
        const node_pos_t node_index = add_node(node);
        std::vector<DagGraph::edge_descriptor> edges_to_remove;
        edges_to_remove.reserve(boost::in_degree(end_node_pos, graph));
        std::vector<Edge> edges_to_add;
        edges_to_add.reserve(boost::in_degree(end_node_pos, graph) * 2);
        std::unordered_set<qubit_t> node_remain_qubits(node.qubit_pos.begin(), node.qubit_pos.end());

        DagGraph::in_edge_iterator ei, ei_end;
        for (boost::tie(ei, ei_end) = boost::in_edges(end_node_pos, graph); ei != ei_end; ++ei) {
            const qubit_t qubit = graph[*ei].qubit_id; 
            // 判断这条边的qubit是否在node的qubit_pos中
            if ( std::find(node.qubit_pos.begin(), node.qubit_pos.end(), qubit) != node.qubit_pos.end() ) {
                edges_to_remove.push_back(*ei);
                edges_to_add.emplace_back(boost::source(*ei, graph), node_index, qubit);
                edges_to_add.emplace_back(node_index, end_node_pos, qubit);
                node_remain_qubits.erase(qubit);
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
