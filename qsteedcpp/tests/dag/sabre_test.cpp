#include <gtest/gtest.h>
#include <boost/graph/graph_utility.hpp>
#include <iostream>
#include "sabre_layout.h"
#include "dag.h"
#include "Model/coupling.h"

class SabreTest : public ::testing::Test {
protected:
    DAGCircuit CreateTestDAG() {
        DagGraph test_graph;
        DAGCircuit dag{test_graph};
        dag.add_instruction_node_end(InstructionNode{"a", 0});
        dag.add_instruction_node_end(InstructionNode{"barrier", 1});
        
        dag.add_edge(0, 3, 2);
        dag.add_edge(0, 3, 3);
        dag.add_edge(3, 1, 2);
        dag.add_edge(3, 1, 3);

        return dag;
    }

    CouplingCircuit CreateTestCouplingCircuit() {
        CouplingList c_list = {
            {0, 1, 0.982f}, {1, 0, 0.982f}, {1, 2, 0.982f}, {2, 1, 0.982f}, {2, 3, 0.982f}, {3, 2, 0.982f}, {2, 4, 0.982f}, {4, 2, 0.982f},
        };

        CouplingCircuit c_circuit{c_list};
        return c_circuit;
    }
};

TEST_F(SabreTest, DAGCreationTest) {
    DAGCircuit dag = CreateTestDAG();
    
    // Test num_qubits
    EXPECT_EQ(dag.num_qubits(), 4);
    
    // Test edges from start node
    DagGraph::edge_iterator ei, ei_end;
    int edge_count = 0;
    for (boost::tie(ei, ei_end) = boost::edges(dag.graph); ei != ei_end; ++ei) {
        if (boost::source(*ei, dag.graph) == dag.start_node_pos) {
            std::cout << "Edge: " << *ei << std::endl;
            edge_count++;
        }
    }
    EXPECT_GT(edge_count, 0);
}

TEST_F(SabreTest, CouplingCircuitCreationTest) {
    CouplingCircuit c_circuit = CreateTestCouplingCircuit();
    
    // Test coupling circuit creation
    // The original test just prints, so we verify it doesn't throw
    EXPECT_NO_THROW(c_circuit.print());
}

TEST_F(SabreTest, SabreRoutingTest) {
    CouplingCircuit c_circuit = CreateTestCouplingCircuit();
    DAGCircuit dag = CreateTestDAG();
    
    sabre::SabreRouting sabre_routing{c_circuit};
    EXPECT_NO_THROW(sabre_routing.run(dag));
}

TEST_F(SabreTest, SabreLayoutTest) {
    CouplingCircuit c_circuit = CreateTestCouplingCircuit();
    sabre::SabreLayout sabre_layout{c_circuit, 3, "fidelity"};
    DAGCircuit dag = CreateTestDAG();
    
    EXPECT_NO_THROW(sabre_layout.run(dag));
}