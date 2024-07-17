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

"""Here are some examples of functions commonly used in tests"""

from qsteed.backends.backend import Backend
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.model import Model
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.passflow.passflow import PassFlow
from qsteed.utils.random_circuit import RandomCircuit

# Test unified parameter settings
coupling_list = [(2, 3, 0.982), (3, 2, 0.982), (3, 4, 0.973), (4, 3, 0.973),
                 (0, 1, 0.98), (1, 0, 0.98), (1, 2, 0.97), (2, 1, 0.97)]  # qubit numbering starts from 0
qubits = 5  # The number of qubits in the random circuit should match the number of qubits in the chip's coupling_list.
# Specify the basic gate set that the transpiled circuit will ultimately use.
basis_gates = ['cx', 'rx', 'ry', 'rz', 'id', 'h']
# A set of quantum gates used to generate random circuit. Set to None to use all supported quantum gates.
gates_list = ['ccx', 'rxx', 'rz', 'ry', 'h', 'x', 'y', 'z']


def get_random_circuit(gates_number: int = 20, depth: int = None):
    """Only one parameter, either the gates_number or the depth, needs to be specified,
        with preference given to the gates_number if both are provided."""
    rqc = RandomCircuit(num_qubit=qubits, gates_number=gates_number, depth=depth,
                        gates_list=gates_list, measure=True)
    qc = rqc.random_circuit()
    return qc


def get_backend_properties():
    backend_properties = {
        'name': 'ExampleBackend',
        'backend_type': 'superconducting',
        'qubits_num': qubits,
        'coupling_list': coupling_list,
        'basis_gates': basis_gates,
    }
    return backend_properties


def get_initial_model():
    backend_properties = get_backend_properties()
    backend_instance = Backend(**backend_properties)
    initial_model = Model(backend=backend_instance)
    return initial_model


def get_passflow():
    backend_properties = get_backend_properties()
    passes = [
        UnrollTo2Qubit(),
        SabreLayout(heuristic='fidelity', max_iterations=3),  # heuristic='distance' or 'fidelity', 'mixture'
        UnrollToBasis(basis_gates=backend_properties['basis_gates']),
        GateCombineOptimization(),
        OneQubitGateOptimization(),
    ]
    passflow = PassFlow(passes=passes)
    return passflow
