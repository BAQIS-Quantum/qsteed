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


from qsteed.apis.compiler_api import call_compiler_api


class TestCallCompiler:
    """Test cases for call_compiler_api."""

    def test_call_compiler_api(self):
        """Test call_compiler_api ."""
        qasm = """
        OPENQASM 2.0;
        include "qelib1.inc";
        qreg q[4];
        creg meas[4];
        h q[0];
        cx q[0],q[1];
        cx q[1],q[2];
        cx q[2],q[3];
        barrier q[0],q[1],q[2],q[3];
        measure q[0] -> meas[0];
        measure q[1] -> meas[1];
        measure q[2] -> meas[2];
        measure q[3] -> meas[3];
        """
        task_info = {
            "circuit": qasm,
            "transpile": True,
            "qpu_name": 'baihua',
            "optimization_level": 3,
            "task_type": 'qc',
            "vqpu_preferred": 'priority'
        }
        compiled_info = call_compiler_api(**task_info)

        assert compiled_info is not None

        print('Compiled openqasm:\n', compiled_info[0])
        print('Measurement qubits to cbits:\n', compiled_info[1])
        print('Compiled circuit information:\n', compiled_info[2])


if __name__ == "__main__":
    t = TestCallCompiler()
    t.test_call_compiler_api()
