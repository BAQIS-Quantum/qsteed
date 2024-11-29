#pragma once
#include <string>
#include "coupling.h"
#include "model.h"
#include "sabre_routing.h"
#include "DAG/dag.h" 
#include "layout.h"

namespace sabre {

    class SabreLayout
    {
    public:
        CouplingCircuit c_circuit;
        std::shared_ptr<Model> model;
        std::unique_ptr<SabreRouting> routing;
        Heuristic heuristic = Heuristic::DISTANCE;
        int max_iterations = 3; 

    public:
        SabreLayout(const CouplingCircuit& c_circuit);
        SabreLayout(const CouplingCircuit& c_circuit, Heuristic heuristic);

        void set_model(const Model& model) { 
            this->model = std::make_shared<Model>(model); 
        }

        Model get_model() const { 
            return *(this->model); 
        }

        DAGCircuit run(const DAGCircuit& dag) {
            // std::cout << "Run sabre layout !" << std::endl;
            // Precheck
            std::set<int> qubits_used = dag.get_qubits_used();
            if (qubits_used.size() == 1) { 
                std::cerr << "Warning: single qubit circuit no need optimize." << std::endl;
                return dag; 
            }
            if (qubits_used.size() > this->c_circuit.num_qubits) {
                throw std::runtime_error("More virtual qubits than physical qubits.");
            }

            if (model->init_layout.empty()) {
                // std::cout << "empty layout" << std::endl;
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
            // print_layout(this->model->init_layout.get_p2v());


            // Add measurement, i.e. mapping of physical qubits to classic qubits
            std::unordered_map<qubit_t, cbit_t> p2c = {};  
            for (const auto& [v, c] : dag.measure) {
                p2c[this->model->final_layout.get_v2p().at(v)] = c;
            }
            physical_dag.measure = p2c;  


            return physical_dag;
        }


        DAGCircuit run_single(const DAGCircuit& dag) {
            DAGCircuit new_dag = this->routing->run(dag);
            return new_dag;
        }
    };

};