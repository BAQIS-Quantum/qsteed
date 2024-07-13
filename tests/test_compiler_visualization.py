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
import matplotlib.pyplot as plt

from qsteed.backends.backend import Backend
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.model import Model
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.transpiler.transpiler_visualization import TranspilerVis, dynamic_draw, \
    draw_allpass_circuits
from qsteed.utils.random_circuit import RandomCircuit
from qsteed.passflow.passflow import PassFlow

rqc = RandomCircuit(num_qubit=5, gates_number=1000, gates_list=None)
qc = rqc.random_circuit()
# qc.plot_circuit()
# plt.show()
qc.draw_circuit()

basis_gates = ['cx', 'rx', 'ry', 'rz', 'id', 'h', 'swap']
c_list = [(0, 1, 0.95), (1, 0, 0.95), (1, 2, 0.99), (2, 3, 0.96), (2, 1, 0.99), (3, 2, 0.96), (3, 4, 0.9), (4, 3, 0.9)]
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

compiler_vis = TranspilerVis(passflow, initial_model)
compiled_circuit_vis, info, short_info = compiler_vis.transpile_vis(qc)

# draw the circuit before and after the input index pass, here the input index is dynamic
print("The information about the input index pass, here the input index is dynamic")
dynamic_draw(info, short_info)
# dynamic_draw_tabulate(info, short_info)
