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
        SabreLayout(const CouplingCircuit& c_circuit, Heuristic heuristic, int max_iterations);

        void set_model(const Model& model) { this->model = std::make_shared<Model>(model); }

        Model get_model() const { return *(this->model); }

        DAGCircuit run_single(const DAGCircuit& dag);

        DAGCircuit run(const DAGCircuit& dag);
    };

};