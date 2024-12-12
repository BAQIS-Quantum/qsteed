#include "sabre_layout.h"
#include "sabre_routing.h"


namespace sabre { 

SabreLayout::SabreLayout(const CouplingCircuit& c_circuit) 
    : c_circuit(c_circuit) {
    // Initialize the model
    this->model = std::make_shared<Model>();

    // Initialize the sabre_routing
    this->routing = std::make_unique<SabreRouting>(c_circuit);

    this->routing->model = this->model;
}

SabreLayout::SabreLayout(const CouplingCircuit& c_circuit, Heuristic heuristic) 
    : c_circuit(c_circuit), heuristic(heuristic) {

    this->model = std::make_shared<Model>();

    // Initialize the sabre_routing
    this->routing = std::make_unique<SabreRouting>(c_circuit, heuristic);

    this->routing->model = this->model;

}

SabreLayout::SabreLayout(const CouplingCircuit& c_circuit, Heuristic heuristic, int max_iterations)
    : SabreLayout(c_circuit, heuristic) {
        this->max_iterations = max_iterations;
}


DAGCircuit SabreLayout::run_single(const DAGCircuit& dag) {
    DAGCircuit new_dag = this->routing->run(dag);
    return new_dag;
}


DAGCircuit SabreLayout::run(const DAGCircuit& dag) {
    std::set<int> qubits_used = dag.get_qubits_used();
    if (qubits_used.size() == 1) { 
        std::cerr << "Warning: single qubit circuit no need optimize." << std::endl;
        return dag; 
    }
    if (qubits_used.size() > this->c_circuit.num_qubits) {
        throw std::runtime_error("More virtual qubits than physical qubits.");
    }

    if (model->init_layout.empty()) {
        model->init_layout = generate_random_layout(qubits_used.size(), c_circuit.num_qubits);
    }
    
    this->routing->modify_dag = false;  // make sure modify_dag = false
    DAGCircuit rev_dag = dag.reverse();

    // Start Iteration 
    for (int i=0; i < max_iterations; i++) {
        for (const auto& direction : {0, 1}) {
            this->run_single(direction == 0 ? dag : rev_dag);
            this->model->init_layout = this->model->final_layout;
        }
    }

    // The last forward iteration obtains the final circuit.
    this->routing->modify_dag = true;
    DAGCircuit physical_dag = this->run_single(dag);

    // Add measurement, i.e. mapping of physical qubits to classic qubits
    std::unordered_map<qubit_t, cbit_t> p2c = {};  
    for (const auto& [v, c] : dag.measure) {
        p2c[this->model->final_layout.get_v2p().at(v)] = c;
    }
    physical_dag.measure = p2c;  

    return physical_dag;
}


};  // namespace sabre