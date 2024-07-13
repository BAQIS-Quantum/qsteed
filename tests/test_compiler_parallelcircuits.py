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

from qsteed.backends.backend import Backend
from qsteed.parallelmanager.parallel_circuits import parallel_process_circuits
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.model import Model
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.transpiler.transpiler import Transpiler
from qsteed.utils.random_circuit import RandomCircuit
from qsteed.passflow.passflow import PassFlow


def create_random_circuits(num_circuits, qubits, depth, gates_list=None):
    circuits = []
    for _ in range(num_circuits):
        rqc = RandomCircuit(num_qubit=qubits, depth=depth, gates_list=gates_list)
        qc = rqc.random_circuit()
        qc.measure([i for i in range(qubits)], [i for i in range(qubits)])
        circuits.append(qc)
    return circuits


if __name__ == '__main__':
    num_processes = None  # Number of processes
    qubits = 5
    depth = 1000
    num_circuits = 4  # Number of circuits in parallel

    # Define quantum circuit compilation passes and model
    basis_gates = ['cx', 'rx', 'ry', 'rz', 'id', 'h']
    c_list = [(0, 1, 0.95), (1, 0, 0.95), (1, 2, 0.99), (2, 3, 0.96), (2, 1, 0.99), (3, 2, 0.96), (3, 4, 0.9),
              (4, 3, 0.9)]
    passes = [
        UnrollTo2Qubit(),
        SabreLayout(heuristic='distance', max_iterations=3),
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

    # Create a list of random circuits and process them in parallel
    circuits = create_random_circuits(num_circuits, qubits, depth)
    t = time.time()
    compiled_circuits = parallel_process_circuits(circuits, passflow, initial_model, num_processes)
    para_time = time.time() - t
    print("Parallel compilation time:", para_time)

    # Serial compilation
    t = time.time()
    for circuit in circuits:
        transpiler = Transpiler(passflow, initial_model)
        compiled_circuit = transpiler.transpile(circuit)
    serial_time = time.time() - t
    print("Serial compilation time:", serial_time)

    print("Serial compilation time/parallel compilation time:", serial_time / para_time)
