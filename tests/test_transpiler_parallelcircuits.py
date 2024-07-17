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

from qsteed.parallelmanager.parallel_circuits import parallel_process_circuits
from qsteed.transpiler.transpiler import Transpiler
from tests.shared_utils import get_passflow, get_initial_model, get_random_circuit


def create_random_circuits(num_circuits, gates_number: int = 20, depth: int = None):
    circuits = []
    for _ in range(num_circuits):
        qc = get_random_circuit(gates_number=gates_number, depth=depth)
        circuits.append(qc)
    return circuits


class TestParallelTranspile:

    def test_parallel_transpile(self):
        """Create a list of random circuits and transpile them in parallel and serial respectively."""
        num_circuits = 4
        gates_number = 20  # Only one parameter, either the gates_number or the depth, needs to be specified,
        depth = 100  # with preference given to the gates_number if both are provided.
        num_processes = None  # Number of processes
        circuits = create_random_circuits(num_circuits, gates_number=gates_number, depth=depth)
        passflow = get_passflow()
        initial_model = get_initial_model()

        # Parallel transpilation
        t = time.time()
        transpiled_circuits = parallel_process_circuits(circuits, passflow, initial_model, num_processes)
        parallel_time = time.time() - t
        assert len(transpiled_circuits) == num_circuits
        for tc in transpiled_circuits:
            assert tc is not None
        print("Parallel transpilation time:", parallel_time)

        # Serial transpilation
        t = time.time()
        for circuit in circuits:
            transpiler = Transpiler(passflow, initial_model)
            transpiled_circuit = transpiler.transpile(circuit)
            assert transpiled_circuit is not None
        serial_time = time.time() - t
        print("Serial transpilation time:", serial_time)

        # Calculate the ratio of serial transpilation time to parallel transpilation time
        print("Serial transpilation time/parallel transpilation time:", serial_time / parallel_time)
