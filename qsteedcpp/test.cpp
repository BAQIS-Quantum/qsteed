#include <iostream>
#include "sabre_layout.h"
#include "dag.h"
#include "Model/coupling.h"
#include "Model/layout.h"
#include "visualization.h"

#include <boost/graph/graph_utility.hpp>
#include <iomanip>

#include "vendor/prettyprint.hpp"


DAGCircuit test_dag() {
    DagGraph test_graph;
    DAGCircuit dag{test_graph};
    dag.add_instruction_node_end(InstructionNode{"a", 0});
    dag.add_instruction_node_end(InstructionNode{"barrier", 1});

    
    dag.add_edge(0, 3, 2);
    dag.add_edge(0, 3, 3);
    dag.add_edge(3, 1, 2);
    dag.add_edge(3, 1, 3);


    DagGraph::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(dag.graph); ei != ei_end; ++ei) {
        if ( boost::source(*ei, dag.graph) == dag.start_node_pos) {
            std::cout << "Edge: " << *ei << std::endl;
        }
    }
    std::cout << dag.num_qubits() << std::endl;
    // dag.draw();


    return dag;
}

CouplingCircuit test_c_ciruit() {
    // CouplingList c_list  = {
    //     {0, 1, 0.98f}, {1, 0, 0.98f}, {1, 2, 0.97f}, {2, 1, 0.97f},{2, 3, 0.982f}, {3, 2, 0.982f},
    // };

    CouplingList c_list = {
        {2, 3, 0.982f}, {3, 2, 0.982f},
    };

    CouplingCircuit c_circuit{c_list};


    // std::map<std::pair<int, int>, double> fd = c_circuit.get_fidelity_dict();
    // std::vector<int> path = c_circuit.get_mini_path(1,4);
    c_circuit.print();


    return c_circuit;
}

void test_sabre_routing() {
    CouplingCircuit c_circuit = test_c_ciruit();
    DAGCircuit dag = test_dag();

    // c_circuit.draw_self();
    sabre::SabreRouting sabre_routing{c_circuit};
    sabre_routing.run(dag);
}


void test_sabre_layout() {
    CouplingCircuit c_circuit = test_c_ciruit();
    sabre::SabreLayout sabre_layout{c_circuit, 3, "fidelity"};
    DAGCircuit dag = test_dag();
    // Vis::draw_graph(dag.graph);
    sabre_layout.run(dag);

}


int main() {
    std::cout << "---- main function ----" << std::endl;
    test_c_ciruit();
    // test_sabre_routing();
    // test_dag();
    // test_sabre_layout();
}