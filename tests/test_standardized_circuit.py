# This code is part of QSteed.
#
# (C) Copyright 2024 Beijing Academy of Quantum Information Sciences
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from qsteed.compiler.qasm_parser import actually_bits, reset_qasm_bits
from qsteed.compiler.standardized_circuit import StandardizedCircuit


class TestStandardizedCircuit:
    """Test cases for testing StandardizedCircuit."""

    def test_standardized_circuit(self):
        """Test standardized circuit."""
        circuit = """
        OPENQASM 2.0;
        include "qelib1.inc";
        qreg qq[10];
        creg meas[5];
        h qq[3];
        barrier qq[5];
        cx qq[3],qq[5];
        cx qq[5],qq[8];
        cx qq[8],qq[2];
        barrier qq[3];
        measure qq[3] -> meas[0];
        barrier qq[5];
        measure qq[5] -> meas[1];
        barrier qq[8];
        measure qq[8] -> meas[2];
        barrier qq[2];
        measure qq[2] -> meas[3];
        """

        qubits, cbits = actually_bits(circuit)

        # Reset qubit/cbit
        if len(cbits) == 0:
            cbits = qubits
        circuit = reset_qasm_bits(circuit, qubits, cbits)
        print(circuit)

        new_circuit = StandardizedCircuit(circuit)
        new_circuit.standardized_circuit()
        print(new_circuit.circuit)


if __name__ == "__main__":
    t = TestStandardizedCircuit()
    t.test_standardized_circuit()
