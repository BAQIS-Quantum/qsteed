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


from qsteed.compiler.compiler import Compiler


class TestCompiler:
    """Test cases for compiler."""

    def test_compiler(self):
        """Test compiler ."""
        qasm = """
        OPENQASM 2.0;
        include "qelib1.inc";
        qreg q[10];
        creg meas[4];
        h q[3];
        cx q[3],q[5];
        cx q[5],q[8];
        cx q[8],q[2];
        barrier q[3],q[5],q[8],q[2];
        measure q[3] -> meas[0];
        measure q[5] -> meas[1];
        measure q[8] -> meas[2];
        measure q[2] -> meas[3];
        """
        compiler = Compiler(qasm, qpu_name='example', optimization_level=1, transpile=True)
        compiled_openqasm, measure_q2c, compiled_circuit_information = compiler.compile()

        assert compiled_openqasm is not None
        assert measure_q2c is not None
        assert compiled_circuit_information is not None

        print('Compiled openqasm:\n', compiled_openqasm)
        print('Measurement qubits to cbits:\n', measure_q2c)
        print('Compiled circuit information:\n', compiled_circuit_information)


if __name__ == "__main__":
    t = TestCompiler()
    t.test_compiler()
