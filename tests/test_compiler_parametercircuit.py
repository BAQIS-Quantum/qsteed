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

import time

from quafu import QuantumCircuit
from quafu.elements.parameters import Parameter

from qsteed.backends.backend import Backend
from qsteed.passes.ParameterTuning.parametersubstitution import ParaSubstitution
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.model import Model
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.transpiler.transpiler import Transpiler
from qsteed.passflow.passflow import PassFlow


def order_variables_indices(circuit: QuantumCircuit, initial_variables: list = None):
    circuit.get_parameter_grads()
    transpiled_variables = circuit._variables
    initial_order = {item.name: i for i, item in enumerate(initial_variables)}
    order_indices = [initial_order[x.name] for x in transpiled_variables]
    return order_indices


pq = QuantumCircuit(5)
theta = [Parameter("theta_%d" % (i), 0.1) for i in range(10)]
theta[4].value = 3.3
pq.rx(1, 0.8)
pq.rx(1, theta[2] * theta[4] * theta[3])
pq.rx(1, 2 * theta[4])
pq.ry(2, theta[5]+0.9)
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

basis_gates = ['cx', 'rx', 'ry', 'rz', 'id', 'h']
c_list = [(2, 3, 0.982), (3, 2, 0.982), (3, 4, 0.973), (4, 3, 0.973),
          (0, 1, 0.98), (1, 0, 0.98), (1, 2, 0.97), (2, 1, 0.97)]

passes = [
    UnrollTo2Qubit(),
    SabreLayout(heuristic='fidelity', max_iterations=3),
    UnrollToBasis(basis_gates=basis_gates),
    GateCombineOptimization(),
    OneQubitGateOptimization(),
    ParaSubstitution(),
]
passflow = PassFlow(passes=passes)

backend_properties = {
    'name': 'ExampleBackend',
    'backend_type': 'superconducting',
    'qubits_num': 5,
    'coupling_list': c_list,
    'basis_gates': basis_gates,
}
backend_instance = Backend(**backend_properties)
initial_model = Model(backend=backend_instance)

st = time.time()
transpiler = Transpiler(passflow, initial_model)
compiled_circuit = transpiler.transpile(pq)
print('compiled circuit:')
compiled_circuit.draw_circuit()

# After transpilation, the parameters order may be changed,
# if we want easily update the parameter in the order of the original parameters,
# we need to get the order map relation between the original parameters and the transpiled parameters
compiled_circuit.get_parameter_grads()
initial_variables = transpiler.model.datadict['variables']
print("initial circuit variables:", initial_variables)
print("compiled circuit variables:", compiled_circuit._variables)
order_indices = order_variables_indices(compiled_circuit, initial_variables)
update_initial_variables = [0, 0, 0, 9, 0, 0, 0, 0, 0, 0]
compiled_circuit._update_params(update_initial_variables, order=order_indices)
print('compiled circuit update parameters:')
compiled_circuit.draw_circuit()
