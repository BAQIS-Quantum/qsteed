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

import numpy as np
from scipy.stats import unitary_group

from qsteed.passes.decomposition.unitary_decompose import UnitaryDecompose
from qsteed.passes.decomposition.utils import convert_utils as cu
from qsteed.passes.decomposition.utils import matrix_utils as mu


class TestUnitaryDecompose:
    def test_unitary_decompose(self):
        # Create random unitary
        qubits = 4
        U0 = unitary_group.rvs(2 ** qubits)

        # Decompose any unitary matrix to obtain quantum circuit
        qubit_list = list(range(qubits))
        ud = UnitaryDecompose(U0, qubit_list, one_qubit_decompose='XZX')
        ud.decompose()
        gates_list = ud.gates_list
        quafuQC = ud.quafuQC
        quafuQC.draw_circuit()

        # Quantum circuit conversion to unitary matrix
        quafuU = cu.circuit_to_unitary(quafuQC)
        assert mu.is_approx(quafuU * np.exp(1j * ud.global_phase),
                            U0), "The decomposed circuit is not consistent with the original unitary"
        print("Is the decomposed circuit consistent with the original unitary?",
              mu.is_approx(quafuU * np.exp(1j * ud.global_phase), U0))

        # Quantum gates list conversion to unitary matrix
        du = cu.gates_list_to_unitary(qubits, gates_list)
        assert mu.is_approx(du * np.exp(1j * ud.global_phase),
                            U0), "The decomposed circuit is not consistent with the original unitary"
        print("Is the decomposed circuit consistent with the original unitary?",
              mu.is_approx(du * np.exp(1j * ud.global_phase), U0))

        # Calculate the distance between the original and decomposed unitary
        distance = mu.matrix_distance_squared(quafuU, U0)
        assert distance < 1e-9, f"The distance between the original and decomposed unitary is too large: {distance}"
        print('The distance between the original and decomposed unitary:', distance)
