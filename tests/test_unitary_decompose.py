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
from quafu.elements.matrices import ryy_mat

qubits = 4
qubit_list = list(range(qubits))
U0 = unitary_group.rvs(2 ** qubits)

ud = UnitaryDecompose(U0, qubit_list, one_qubit_decompose='XZX')
ud.decompose()
gates_list = ud.gates_list
quafuQC = ud.quafuQC
quafuQC.draw_circuit()

quafuU = cu.circuit_to_unitary(quafuQC)
print("Is the decomposed circuit consistent with the original unitary?", mu.is_approx(quafuU * np.exp(1j * ud.global_phase), U0))

du = cu.gates_list_to_unitary(qubits, gates_list)
print("Is the decomposed circuit consistent with the original unitary?", mu.is_approx(du * np.exp(1j * ud.global_phase), U0))

distance = mu.matrix_distance_squared(quafuU, U0)
print('distance', distance)
