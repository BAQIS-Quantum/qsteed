#pragma once
#include <string>
#include "sabre_core.h"
#include "Model/coupling.h"
#include "Model/model.h"
#include "Model/layout.h"
#include "sabre_routing.h"
#include "DAG/dag.h" 

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
        SabreLayout(const CouplingCircuit& c_circuit, int max_iterations, const std::string& heuristic_str);
        SabreLayout(const CouplingCircuit& c_circuit, int max_iterations, const std::string& heuristic_str, LayoutStructure initial_layout);

        void set_model(const Model& model) { this->model = std::make_shared<Model>(model); }

        Model get_model() const { return *(this->model); }

        int get_add_swap_count() const { return this->routing->get_add_swap_count(); }

        DAGCircuit run_single(const DAGCircuit& dag);

        DAGCircuit run(const DAGCircuit& dag);
    };

};