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
        task_info = {
            "circuit": qasm,
            "transpile": True,
            "qpu_name": 'example',
            "optimization_level": 2,
            "task_type": 'qc',
        }
        compiled_info = call_compiler_api(**task_info)

        assert compiled_info is not None

        print('Compiled openqasm:\n', compiled_info[0])
        print('Measurement qubits to cbits:\n', compiled_info[1])
        print('Compiled circuit information:\n', compiled_info[2])


if __name__ == "__main__":
    t = TestCallCompiler()
    t.test_call_compiler_api()
