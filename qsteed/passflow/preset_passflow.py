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

from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from .passflow import PassFlow


def level_0_passflow(basis_gates):
    level_0 = [UnrollTo2Qubit(),
               UnrollToBasis(basis_gates=basis_gates)
               ]
    return PassFlow(level_0)


def level_1_passflow(basis_gates):
    level_1 = [UnrollTo2Qubit(),
               SabreLayout(heuristic='distance', max_iterations=3),
               UnrollToBasis(basis_gates=basis_gates),
               GateCombineOptimization(),
               # OneQubitGateOptimization(),
               ]
    return PassFlow(level_1)


def level_2_passflow(basis_gates):
    level_2 = [UnrollTo2Qubit(),
               SabreLayout(heuristic='fidelity', max_iterations=3),
               UnrollToBasis(basis_gates=basis_gates),
               GateCombineOptimization(),
               # OneQubitGateOptimization(),
               ]
    return PassFlow(level_2)


def level_3_passflow(basis_gates):
    level_3 = [UnrollTo2Qubit(),
               SabreLayout(heuristic='mixture', max_iterations=3),
               UnrollToBasis(basis_gates=basis_gates),
               GateCombineOptimization(),
               # OneQubitGateOptimization(),
               ]
    return PassFlow(level_3)


class PresetPassflow(PassFlow):
    """A qsteed passflow describes a quantum circuit compilation process."""

    def __init__(self, basis_gates, optimization_level=0):
        super().__init__()
        self.basis_gates = basis_gates
        self.optimization_level = optimization_level

    def get_passflow(self):
        if self.optimization_level == 0:
            return level_0_passflow(self.basis_gates)
        elif self.optimization_level == 1:
            return level_1_passflow(self.basis_gates)
        elif self.optimization_level == 2:
            return level_2_passflow(self.basis_gates)
        elif self.optimization_level == 3:
            return level_3_passflow(self.basis_gates)
        else:
            raise ValueError("Error: The value of optimization_level is between [0,3].")
