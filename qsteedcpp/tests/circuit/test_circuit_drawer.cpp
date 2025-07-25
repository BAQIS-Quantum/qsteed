#include <gtest/gtest.h>
#include "circuit/quantum_circuit.h"

using namespace qsteedcpp;

TEST(CircuitDrawerTest, BasicCircuit) {
    QuantumCircuit qc(3, 2);

    Parameter theta = Parameter("θ");
    
    qc.h(0);
    qc.cnot(0, 1);
    qc.rzz(theta, 0, 2);
    qc.rxx(theta, 0, 1);
    qc.swap(0, 2);
    qc.cnot(0, 1);
    qc.cnot(0, 1);
    qc.x(2);
    qc.rxx(theta, 0, 1);
    qc.rxx(theta, 0, 1);
    qc.cnot(0, 1);
    qc.measure(0, 0);
    qc.measure(1, 1);
    
    qc.print();
    // qc.print_interactive();
}