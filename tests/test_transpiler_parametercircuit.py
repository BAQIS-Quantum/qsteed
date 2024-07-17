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

from quafu import QuantumCircuit
from quafu.elements.parameters import Parameter

from qsteed.passes.ParameterTuning.parametersubstitution import ParaSubstitution
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.passflow.passflow import PassFlow
from qsteed.transpiler.transpiler import Transpiler
from tests.shared_utils import get_initial_model


def order_variables_indices(circuit: QuantumCircuit, initial_variables: list = None):
    circuit.get_parameter_grads()
    transpiled_variables = circuit._variables
    initial_order = {item.name: i for i, item in enumerate(initial_variables)}
    order_indices = [initial_order[x.name] for x in transpiled_variables]
    return order_indices


class TestParameterizedCircuitTranspile:

    def test_parameterized_circuit_transpile(self):
        # Create parameterized circuit
        pq = QuantumCircuit(5)
        theta = [Parameter("theta_%d" % i, 0.1) for i in range(10)]
        theta[4].value = 3.3
        pq.rx(1, 0.8)
        pq.rx(1, theta[2] * theta[4] * theta[3])
        pq.rx(1, 2 * theta[4])
        pq.ry(2, theta[5] + 0.9)
        pq.rx(3, theta[6])
        pq.rx(4, theta[7])
        pq.rxx(0, 3, theta[0])
        pq.rzz(2, 3, theta[8])
        pq.rzz(0, 2, theta[4])
        pq.rx(0, theta[9])
        pq.rx(0, 3.5)
        pq.cx(3, 4)
        pq.ry(2, theta[1].sin() - 4. * theta[0] + theta[2] * theta[0] + 2.5)
        pq.rx(2, theta[1] - 4. * theta[0] + theta[2] * theta[0])
        pq.measure([0, 1, 2, 3, 4], [0, 1, 2, 3, 4])
        print("original circuit:")
        pq.draw_circuit()

        passes = [
            UnrollTo2Qubit(),
            SabreLayout(heuristic='fidelity', max_iterations=3),
            UnrollToBasis(basis_gates=['cx', 'rx', 'ry', 'rz', 'id', 'h']),
            GateCombineOptimization(),
            OneQubitGateOptimization(),
            ParaSubstitution(),  # Parameterized pass
        ]
        passflow = PassFlow(passes=passes)
        initial_model = get_initial_model()

        transpiler = Transpiler(passflow, initial_model)
        transpiled_circuit = transpiler.transpile(pq)
        assert transpiled_circuit is not None
        transpiled_circuit.draw_circuit()

        initial_variables = transpiler.model.datadict['variables']
        assert initial_variables is not None
        print("Initial circuit variables:", initial_variables)
        print("Compiled circuit variables:", transpiled_circuit._variables)

        # After transpilation, the parameters order may be changed,
        # if we want easily update the parameter in the order of the original parameters,
        # we need to get the order map relation between the original parameters and the transpiled parameters
        transpiled_circuit.get_parameter_grads()
        order_indices = order_variables_indices(transpiled_circuit, initial_variables)
        assert order_indices is not None

        update_initial_variables = [0, 0, 0, 9, 0, 0, 0, 0, 0, 0]
        transpiled_circuit._update_params(update_initial_variables, order=order_indices)
        print('Compiled circuit updated parameters:')
        transpiled_circuit.draw_circuit()
