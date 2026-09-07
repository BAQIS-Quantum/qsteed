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

from math import pi

import numpy as np
from quafu import QuantumCircuit
from quafu.elements.element_gates import HGate, RZGate, SwapGate

from qsteed.passes.unroll.rules.h2rxry import HToRXRY
from qsteed.passes.unroll.rules.rz2rxry import RZToRXRY
from qsteed.passes.unroll.rules.rz2sxry import RZToSXRY
from qsteed.passes.unroll.rules.swap2czh import SwapToCZH
from qsteed.passes.unroll.rules.swap2czrxry import SwapToCZRXRY
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis


def _sequence_unitary(gates, qubit_num):
    unitary = np.eye(2 ** qubit_num, dtype=complex)
    identity = np.eye(2, dtype=complex)
    for gate in gates:
        gate_matrix = gate.matrix
        if qubit_num == 2 and gate_matrix.shape == (2, 2):
            if gate.pos[0] == 0:
                gate_matrix = np.kron(gate_matrix, identity)
            else:
                gate_matrix = np.kron(identity, gate_matrix)
        unitary = gate_matrix @ unitary
    return unitary


def _assert_equal_up_to_global_phase(actual, expected):
    phase = np.trace(expected.conj().T @ actual) / expected.shape[0]
    assert np.isclose(abs(phase), 1.0)
    assert np.allclose(actual, phase * expected)


def _unroll(gate, basis_gates, qubit_num):
    circuit = QuantumCircuit(qubit_num)
    circuit.add_gate(gate)
    return UnrollToBasis(basis_gates).run(circuit)


def test_new_single_qubit_rules_are_unitary_equivalent():
    theta = 0.37

    h_rule = HToRXRY()
    h_unitary = _sequence_unitary(h_rule.run(HGate(0)), 1)
    assert np.allclose(np.exp(1j * h_rule.global_phase) * h_unitary, HGate(0).matrix)

    rz_rxry = RZToRXRY()
    rz_rxry_unitary = _sequence_unitary(rz_rxry.run(RZGate(0, theta)), 1)
    assert np.allclose(rz_rxry_unitary, RZGate(0, theta).matrix)

    rz_sxry = RZToSXRY()
    rz_sxry_unitary = _sequence_unitary(rz_sxry.run(RZGate(0, theta)), 1)
    assert np.allclose(rz_sxry_unitary, RZGate(0, theta).matrix)


def test_new_swap_rules_are_unitary_equivalent():
    expected = SwapGate(0, 1).matrix

    swap_czh = SwapToCZH()
    czh_unitary = _sequence_unitary(swap_czh.run(SwapGate(0, 1)), 2)
    _assert_equal_up_to_global_phase(czh_unitary, expected)

    swap_czrxry = SwapToCZRXRY()
    czrxry_unitary = _sequence_unitary(swap_czrxry.run(SwapGate(0, 1)), 2)
    assert np.allclose(np.exp(1j * swap_czrxry.global_phase) * czrxry_unitary, expected)


def test_unroll_to_basis_selects_the_matching_new_rule():
    cases = (
        (HGate(0), ['rx', 'ry'], 1, {'rx', 'ry'}),
        (HGate(0), ['ry', 'rz'], 1, {'ry', 'rz'}),
        (RZGate(0, pi / 7), ['rx', 'ry'], 1, {'rx', 'ry'}),
        (RZGate(0, pi / 7), ['sx', 'sxdg', 'ry'], 1, {'sx', 'sxdg', 'ry'}),
        (SwapGate(0, 1), ['cz', 'h'], 2, {'cz', 'h'}),
        (SwapGate(0, 1), ['cz', 'rx', 'ry'], 2, {'cz', 'rx', 'ry'}),
        (SwapGate(0, 1), ['cz', 'ry', 'rz'], 2, {'cz', 'ry', 'rz'}),
        (SwapGate(0, 1), ['cx'], 2, {'cx'}),
    )

    for gate, basis, qubit_num, expected_names in cases:
        unrolled = _unroll(gate, basis, qubit_num)
        assert {item.name.lower() for item in unrolled.gates}.issubset(expected_names)
