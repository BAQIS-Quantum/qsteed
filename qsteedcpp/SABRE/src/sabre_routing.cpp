#include <sabre_routing.h>
#include <iostream>
#include <unordered_map>
#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include "Model/layout.h"
#include "sabre_routing.h"

#include "vendor/prettyprint.hpp"

namespace sabre {

constexpr int SWAP_CANDIDATES_RESERVE = 16;
constexpr int FIDELITY_TIED_RESERVE = 8;
constexpr double INVALID_SWAP_SCORE = -1000000;

/*
    Return a @c DAGCircuit : The original dag or the mapped_dag with added swap gate depending on modify_flag.
*/
DAGCircuit SabreRouting::run(const DAGCircuit& dag) {


    // Precheck
    std::set<int> qubits_used = dag.get_qubits_used();
    if (qubits_used.size() == 1) { 
        std::cerr << "Warning: single qubit circuit no need optimize." << std::endl;
        return dag; 
    }

    if (qubits_used.size() > this->c_circuit.num_qubits) {
        throw std::runtime_error("More virtual qubits than physical qubits.");
    }

    this->add_swap_count = 0;

    for(const auto& qubit : qubits_used)
        this->qubits_decay[qubit] = 1;

    // Size of lookahead window. Set to number of qubits
    this->extended_set_size = this->c_circuit.num_qubits;


    // Parameter preparation before iteration.
    DAGCircuit mapped_dag;

    Layout current_layout;
    if (model->initial_layout.empty())
        model->initial_layout = generate_random_layout(qubits_used.size(), c_circuit.num_qubits);

    current_layout = model->initial_layout;

    std::unordered_map<int, int> pre_executed_counts;
    std::unordered_set<int> front_layer; 
    Matrix distance_matrix = c_circuit.get_distance_matrix();      

    // Initialize the front layer.
    DagGraph::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(dag.graph); ei != ei_end; ++ei) {
        if ( boost::source(*ei, dag.graph) == dag.start_node_pos) {
            const auto target = boost::target(*ei, dag.graph);
            pre_executed_counts[target]++;
            if (dag.graph[target].name != "barrier" && dag.graph[target].name != "measure") {
                if ((pre_executed_counts[target] == 2) || (dag.graph[target].qubit_pos.size() == 1)) {
                    front_layer.insert(target);
                }
            }
        }
    }


    std::vector<std::pair<int, int>> executed_2gate_list;  // The hardware execution order list of executable 2-qubit gates under current_layout.
    std::set<std::pair<int, int>> unavailable_2qubits;

    // Start the algorithm from the front layer and iterate until all gates are completed.    
    int iteration_count = 0;
    while ( !front_layer.empty() ) {
        std::vector<int> execute_gate_list;
        execute_gate_list.reserve(front_layer.size());
        /* process front_layer nodes, record
            executable_gates        -> (单比特的门，barrier，XY门，measure) + (layout后两个qubit直接相连的两比特门) = 直接可执行的门
            unabailable_2quibits    -> unavailable 2-qubits
        */
        for (const auto& node_index : front_layer) {
            const InstructionNode& node = dag.graph[node_index];
            // if the gate is a 2-qubit gate
            if (node.qubit_pos.size() == 2 && node.name != "barrier" && node.name != "XY" && node.name != "measure") {
                int v0 = node.qubit_pos[0];
                int v1 = node.qubit_pos[1];
                int p0 = current_layout[v0];
                int p1 = current_layout[v1];

                // if the two qubits are directly connected in the physical layout
                if (boost::edge(p0, p1, c_circuit.graph).second) {
                    execute_gate_list.push_back(node_index);

                    auto it = unavailable_2qubits.begin();
                    while (it != unavailable_2qubits.end()) {
                        if (it->first == p0 || it->second == p0 || it->first == p1 || it->second == p1) {
                            it = unavailable_2qubits.erase(it);
                        } else {
                            ++it;
                        }
                    }
                }
            } 
            else 
                execute_gate_list.push_back(node_index); // Single-qubit gates, barriers, XY-gates and measures are both executable gates.
        }


        if ( !execute_gate_list.empty() ) {
            for (const int& node_index : execute_gate_list) {
                _apply_gate(mapped_dag, dag.graph[node_index], current_layout);
                front_layer.erase(node_index);

                for ( int successor : _dag_successors(dag, node_index) ) {
                    pre_executed_counts[successor]++;
                    if (pre_executed_counts[successor] == dag.graph[successor].qubit_pos.size()) {
                        front_layer.insert(successor);  
                    }
                }
            }
            iteration_count = 0;
            _reset_qubits_decay();

            continue;
        }


        std::set<int> extended_set = _calc_extended_set(dag, front_layer);

        // Add swap gate
        std::set<SwapPos> swap_candidates = _obtain_swaps(front_layer, current_layout, dag);
        const SwapPos best_swap = _get_best_swap(dag, swap_candidates, current_layout, front_layer, extended_set, unavailable_2qubits); 
        const InstructionNode swap_gate = InstructionNode("swap", std::vector<qubit_t>{best_swap.first, best_swap.second}); 
        _apply_gate(mapped_dag, swap_gate, current_layout);
        this->add_swap_count++;
        current_layout.swap(best_swap.first, best_swap.second);

        // Update excute_gate_list, unavailable_2qubits
        int min_val = std::min(current_layout[best_swap.first], current_layout[best_swap.second]);
        int max_val = std::max(current_layout[best_swap.first], current_layout[best_swap.second]);
        executed_2gate_list.push_back({min_val, max_val});
        unavailable_2qubits.insert({min_val, max_val});


        // Update qubits_decay
        iteration_count++;
        if (iteration_count % this->decay_reset_interval == 0) {
            _reset_qubits_decay();
        } 
        else {
            qubits_decay[best_swap.first] += decay_delta;
            qubits_decay[best_swap.second] += decay_delta;
        } 
    }

    // Update model
    this->model->final_layout = current_layout;


    if (this->modify_dag)   {
        return mapped_dag;
    } else {
        return dag;
    }
}

/**
 * Calculate the extended set for lookahead capabilities.
 *
 * @param dag (DAGCircuit) A DAGCircuit representing the quantum circuit.
 * @param front_layer (list) A vector representing the front layer in the DAG.
 * @return extended_set (set) A set of expansion gates obtained according to requirements.
 */
std::set<int> SabreRouting::_calc_extended_set(const DAGCircuit& dag, const std::unordered_set<int>& front_layer) {
    std::set<int> extended_set{};
    if (front_layer.empty()) {
        return extended_set;
    }

    std::vector<int> queue;
    queue.reserve(this->extended_set_size * 2);  // 预分配队列容量
    queue.assign(front_layer.begin(), front_layer.end());
    std::set<int> visited_nodes(front_layer.begin(), front_layer.end());
    
    size_t head = 0;
    while (head < queue.size() && extended_set.size() < this->extended_set_size) {
        int node_index = queue[head++];

        auto out_edges = boost::out_edges(node_index, dag.graph);
        for (auto it = out_edges.first; it != out_edges.second; ++it) {
            int successor_index = boost::target(*it, dag.graph);
            const auto& successor_node = dag.graph[successor_index];

            if (successor_node.qubit_pos.size() == 2 && successor_node.name != "barrier" && successor_node.name != "measure") {
                extended_set.insert(successor_index);
            }

            if (visited_nodes.find(successor_index) == visited_nodes.end()) {
                if (successor_node.name != "barrier" && successor_node.name != "measure") {
                    queue.push_back(successor_index);
                    visited_nodes.insert(successor_index);
                }
            }
        }
    }
    return extended_set;
}


std::set<SwapPos> SabreRouting::_obtain_swaps(  const std::unordered_set<int>& front_layer, 
                                                const Layout& current_layout, 
                                                const DAGCircuit& dag ) {   
    std::set<SwapPos> candiate_swaps{};
    for ( const auto& node_index : front_layer ) {
        for ( const auto& virtual_pos : dag.graph[node_index].qubit_pos ) {
            int physical_pos = current_layout[virtual_pos];
            auto neighbors = boost::adjacent_vertices(physical_pos, c_circuit.graph);

            for ( auto it =  neighbors.first; it != neighbors.second; ++it ) {
                int virtual_neighbor = current_layout.get_p2v().at(*it);
                SwapPos swap = std::minmax(virtual_pos, virtual_neighbor);
                candiate_swaps.insert(swap);
            }
        }
    }
    return candiate_swaps;
}


/**
 * Get the best swap based on different heuristics.
 *
 * Args:
 *     swap_candidates (set): The set of all candidate swap gates.
 *     current_layout (Layout): current layout
 *     front_layer (list): front layer gates list
 *     extended_set (set): set of expansion gates
 *     unavailable_2qubits (set): set of unavailable two-qubits
 * Returns:
 *     best_swap (tuple): the best swap based on different heuristics
 */
SwapPos SabreRouting::_get_best_swap(   const DAGCircuit& dag,
                                        const std::set<SwapPos>& swap_candidates, 
                                        const Layout& current_layout,
                                        const std::unordered_set<int>& front_layer, 
                                        const std::set<int>& extended_set, 
                                        const std::set<std::pair<int, int>>& unavailable_2qubits) const {

    std::unordered_map<SwapPos, double, sabre::SwapPosHash> swap_scores;
    for ( const auto& swap : swap_candidates ) {
        swap_scores[swap] = INVALID_SWAP_SCORE;
    }

    if ( this->heuristic == Heuristic::FIDELITY ) {
        for ( const auto& swap : swap_candidates )  {
            SwapPos physical_swap = std::minmax(current_layout[swap.first], current_layout[swap.second]);
            if (unavailable_2qubits.find(physical_swap) == unavailable_2qubits.end()) {
                double swap_cost = _swap_score(physical_swap);
                double score_h= _score_heuristic(
                    dag, this->heuristic, front_layer, extended_set, current_layout, swap
                );
                double score = swap_cost + score_h;
                swap_scores[swap] = score;
            }
        }
        auto best_swap = std::max_element(swap_scores.begin(), swap_scores.end(), 
            [](const std::pair<SwapPos, double>& a, const std::pair<SwapPos, double>& b) {
                return a.second < b.second;
            }
        );

        // return best_swap->first;

        std::vector<SwapPos> best_swaps;
        best_swaps.reserve(SWAP_CANDIDATES_RESERVE);
        for (const auto& pair : swap_scores) {
            if (pair.second == best_swap->second) {
                best_swaps.push_back(pair.first);
            }
        }

        std::uniform_int_distribution<> dis(0, best_swaps.size() - 1);

        return best_swaps[dis(gen)];
    }
    else if ( this->heuristic == Heuristic::DISTANCE ) {
        for ( const auto& swap : swap_candidates) {
            double score = _score_heuristic(dag, this->heuristic, front_layer, extended_set, current_layout, swap);
            swap_scores[swap] = score;
        }

        if (swap_scores.empty()) {
            throw std::runtime_error("Sabre routing failed: could not find a valid SWAP for distance heuristic.");
        }

        auto best_swap_it = std::min_element(swap_scores.begin(), swap_scores.end(), 
            [](const std::pair<SwapPos, double>& a, const std::pair<SwapPos, double>& b) {
                return a.second < b.second;
            }
        );

        double min_score = best_swap_it->second;
        std::vector<SwapPos> best_swaps;
        best_swaps.reserve(SWAP_CANDIDATES_RESERVE);
        for (const auto& pair : swap_scores) {
            if (pair.second == min_score) {
                best_swaps.push_back(pair.first);
            }
        }

        std::uniform_int_distribution<> dis(0, best_swaps.size() - 1);

        return best_swaps[dis(gen)];
    }
    else if ( this->heuristic == Heuristic::MIXTURE ) {
        for ( const auto& swap : swap_candidates) {
            double score = _score_heuristic(dag, Heuristic::DISTANCE, front_layer, extended_set, current_layout, swap);
            swap_scores[swap] = score;
        }

        if (swap_scores.empty()) {
            throw std::runtime_error("Sabre routing failed: could not find a valid SWAP for mixture heuristic.");
        }

        auto swap_mini_it = std::min_element(swap_scores.begin(), swap_scores.end(), 
            [](const std::pair<SwapPos, double>& a, const std::pair<SwapPos, double>& b) {
                return a.second < b.second;
            }
        );

        double min_dist_score = swap_mini_it->second;
        std::vector<SwapPos> distance_tied_swaps;
        distance_tied_swaps.reserve(SWAP_CANDIDATES_RESERVE);  // 预分配容量
        for (const auto& pair : swap_scores) {
            if (pair.second == min_dist_score) {
                distance_tied_swaps.push_back(pair.first);
            }
        }

        if (distance_tied_swaps.size() == 1) {
            return distance_tied_swaps[0];
        }

        double best_fidelity_score = -std::numeric_limits<double>::infinity();
        std::vector<SwapPos> fidelity_tied_swaps;
        fidelity_tied_swaps.reserve(FIDELITY_TIED_RESERVE);  // 预分配容量，fidelity tied通常更少

        for (const auto& swap : distance_tied_swaps) {
            SwapPos physical_swap = std::minmax(current_layout[swap.first], current_layout[swap.second]);
            if (unavailable_2qubits.find(physical_swap) == unavailable_2qubits.end()) {
                double swap_cost = _swap_score(physical_swap);
                double score_h= _score_heuristic(
                    dag, Heuristic::FIDELITY, front_layer, extended_set, current_layout, swap
                );
                double current_fidelity_score = swap_cost + score_h;

                if (current_fidelity_score > best_fidelity_score) {
                    best_fidelity_score = current_fidelity_score;
                    fidelity_tied_swaps.clear();
                    fidelity_tied_swaps.push_back(swap);
                } else if (current_fidelity_score == best_fidelity_score) {
                    fidelity_tied_swaps.push_back(swap);
                }
            }
        }

        if (!fidelity_tied_swaps.empty()) {
            std::uniform_int_distribution<> dis(0, fidelity_tied_swaps.size() - 1);
            return fidelity_tied_swaps[dis(gen)];
        }
        
        std::uniform_int_distribution<> dis(0, distance_tied_swaps.size() - 1);
        return distance_tied_swaps[dis(gen)];
    }

    return {0,0};
}


double SabreRouting::_score_heuristic(  const DAGCircuit& dag, 
                                        const Heuristic heuristic,
                                        const std::unordered_set<int>& front_layer, 
                                        const std::set<int>& extended_set, 
                                        const Layout& current_layout,
                                        const SwapPos& swap_pos
) const {
    Layout trial_layout = Layout(current_layout);
    trial_layout.swap(swap_pos.first, swap_pos.second);

    double decay_fisrt = qubits_decay.at(swap_pos.first);
    double decay_second = qubits_decay.at(swap_pos.second);


    if ( heuristic == Heuristic::DISTANCE) {
        double front_cost = _compute_distance_cost(dag, front_layer, trial_layout) / static_cast<double>(front_layer.size());
        double extended_cost = 0; 
        if ( !extended_set.empty() ) {
            extended_cost = _compute_distance_cost(dag, extended_set , trial_layout) / static_cast<double>(extended_set.size());
        }
        double total_cost = front_cost + extended_cost * extended_set_weight;
        return total_cost * std::max(decay_fisrt, decay_second);
    } 
    else if ( heuristic == Heuristic::FIDELITY ) {
        double noise_front_cost = _compute_fidelity_cost(dag, front_layer, trial_layout);
        double noise_extended_cost = 0.0;
        if ( !extended_set.empty() ) {
             noise_extended_cost = _compute_fidelity_cost(dag, extended_set, trial_layout);
        }
        double noise_total_cost = noise_front_cost + extended_set_weight * noise_extended_cost;

        return 0.5 * (decay_fisrt + decay_second) * noise_total_cost;
    }
    else {
        throw std::runtime_error("Unrecognized Heuristic type");
    }

    return 0;
}

}; // namespace sabre