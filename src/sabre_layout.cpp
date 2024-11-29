#include "sabre_layout.h"
#include "sabre_routing.h"


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