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

import random
import unittest

from quafu import QuantumCircuit

from qsteed.backends.backend import Backend
from qsteed.passes.mapping.baselayout import Layout
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.mapping.layout.sabre_layout_parallel import SabreLayoutParallel
from qsteed.passes.model import Model


def _line_couplings(first, last, fidelity=0.99):
    forward = [(qubit, qubit + 1, fidelity) for qubit in range(first, last)]
    backward = [(target, source, weight) for source, target, weight in forward]
    return forward + backward


def _test_circuit():
    circuit = QuantumCircuit(3)
    circuit.h(0)
    circuit.x(1)
    circuit.cx(0, 2)
    circuit.cx(1, 2)
    return circuit


class TestSabreExactSubgraph(unittest.TestCase):

    def _assert_exact_connected_routing(self, model, circuit, expected_size):
        used_subgraph = model.get_backend().get_property('used_subgraph')
        final_layout = model.get_layout()['final_layout']
        used_qubits = set(used_subgraph.qubits_list)

        self.assertEqual(used_subgraph.num_qubits, expected_size)
        self.assertTrue(used_subgraph.is_connected())
        self.assertEqual(set(final_layout.p2v), used_qubits)
        self.assertEqual(len(final_layout.p2v), expected_size)
        for gate in circuit.gates:
            if isinstance(gate.pos, list):
                self.assertLessEqual(set(gate.pos), used_qubits)
                if len(gate.pos) == 2:
                    self.assertTrue(used_subgraph.graph.has_edge(*gate.pos))

    def test_n_less_than_device_selects_exact_connected_subgraph(self):
        couplings = _line_couplings(0, 5)
        for method in ('random', 'fidelity', 'dense'):
            with self.subTest(method=method):
                random.seed(4)
                model = Model(Backend(qubits_num=6, coupling_list=couplings))
                layout_pass = SabreLayout(
                    max_iterations=1,
                    initial_layout_method=method,
                )
                layout_pass.set_model(model)

                physical_circuit = layout_pass.run(_test_circuit())

                self._assert_exact_connected_routing(model, physical_circuit, 3)

    def test_parallel_layout_uses_the_same_exact_subgraph_rule(self):
        random.seed(7)
        model = Model(Backend(
            qubits_num=8,
            coupling_list=_line_couplings(0, 7),
        ))
        layout_pass = SabreLayoutParallel(max_iterations=1)
        layout_pass.set_model(model)

        physical_circuit = layout_pass.run(_test_circuit())

        self._assert_exact_connected_routing(model, physical_circuit, 3)

    def test_nonzero_physical_labels_are_supported(self):
        random.seed(9)
        model = Model(Backend(
            qubits_num=6,
            coupling_list=_line_couplings(5, 10),
        ))
        layout_pass = SabreLayout(max_iterations=0)
        layout_pass.set_model(model)

        physical_circuit = layout_pass.run(_test_circuit())

        self._assert_exact_connected_routing(model, physical_circuit, 3)
        self.assertTrue(
            set(model.get_backend().get_property('used_subgraph').qubits_list) <= set(range(5, 11))
        )

    def test_disconnected_explicit_layout_is_rejected(self):
        model = Model(Backend(
            qubits_num=6,
            coupling_list=_line_couplings(0, 5),
        ))
        layout_pass = SabreLayout(
            max_iterations=0,
            sabre_initial_layout=Layout({0: 0, 1: 2, 2: 4}),
        )
        layout_pass.set_model(model)

        with self.assertRaisesRegex(ValueError, 'connected physical subgraph'):
            layout_pass.run(_test_circuit())


if __name__ == '__main__':
    unittest.main()
