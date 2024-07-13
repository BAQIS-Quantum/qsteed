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

import matplotlib.pyplot as plt

from qsteed.backends.backend import Backend
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.model import Model
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.transpiler.transpiler import Transpiler
from qsteed.utils.random_circuit import RandomCircuit
from qsteed.passflow.passflow import PassFlow

rqc = RandomCircuit(num_qubit=5, gates_number=20,
                    gates_list=['ccx', 'rxx', 'rz', 'ry', 'h', 'x', 'y', 'z'], measure=True)
qc = rqc.random_circuit()
qc.plot_circuit()
plt.show()

basis_gates = ['cx', 'rx', 'ry', 'rz', 'id', 'h']
c_list = [(2, 3, 0.982), (3, 2, 0.982), (3, 4, 0.973), (4, 3, 0.973), (0, 1, 0.98), (1, 0, 0.98), (1, 2, 0.97),
          (2, 1, 0.97)]

passes = [
    UnrollTo2Qubit(),
    SabreLayout(heuristic='fidelity', max_iterations=3),  # heuristic='distance' or 'fidelity', 'mixture'
    UnrollToBasis(basis_gates=basis_gates),
    GateCombineOptimization(),
    OneQubitGateOptimization(),
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
compiler = Transpiler(passflow, initial_model)
compiled_circuit = compiler.transpile(qc)
print('qsteed time (s):', time.time() - st)
print('qsteed depth:', len(compiled_circuit.layered_circuit().T) - 1)
print('qsteed swap count', compiler.model.datadict['add_swap_count'])
compiled_circuit.plot_circuit()
plt.show()
# print('initial_layout', transpiler.model.get_layout()['initial_layout'].v2p)
# print('final_layout', transpiler.model.get_layout()['final_layout'].v2p)

# Using preset compilation passflow, see passflow/preset_passflow.py
compiler = Transpiler(initial_model=initial_model)
compiled_circuit = compiler.transpile(qc, optimization_level=1)
compiled_circuit.plot_circuit()
plt.show()
