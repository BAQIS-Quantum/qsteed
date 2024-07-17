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

"""Test the Transpiler class"""

import time

from qsteed.passes.model import Model
from qsteed.passflow.passflow import PassFlow
from qsteed.transpiler.transpiler import Transpiler
from tests.shared_utils import get_passflow, get_random_circuit, get_initial_model


class TestTranspiler:
    """Test cases for the Transpiler class and related functionalities."""

    def test_random_circuit(self):
        """Test the generation of a random quantum circuit."""
        qc = get_random_circuit()
        assert qc is not None
        # qc.plot_circuit()
        # plt.show()
        qc.draw_circuit()

    def test_backend_model(self):
        """Test the generation of the initial backend model."""
        model = get_initial_model()
        assert model is not None
        assert isinstance(model, Model)

    def test_passflow(self):
        """Test the generation of PassFlow."""
        passflow = get_passflow()
        assert passflow is not None
        assert isinstance(passflow, PassFlow)

    def test_transpiler_custom(self):
        """
        Test the custom Transpiler process.
        Compile the quantum circuit using a custom passflow and initial model.
        """
        qc = get_random_circuit()
        st = time.time()
        passflow = get_passflow()
        initial_model = get_initial_model()
        transpiler = Transpiler(passflow, initial_model)
        transpiled_circuit = transpiler.transpile(qc)
        assert transpiled_circuit is not None
        print('qsteed time (s):', time.time() - st)
        print('qsteed depth:', len(transpiled_circuit.layered_circuit().T) - 1)
        print('qsteed swap count', transpiler.model.datadict['add_swap_count'])
        transpiled_circuit.draw_circuit()
        # transpiled_circuit.plot_circuit()
        # plt.show()

    def test_transpiler_preset(self):
        """
        Test the preset optimization level Transpiler process.
        Compile the quantum circuit using a preset optimization level.
        """
        qc = get_random_circuit()
        initial_model = get_initial_model()
        transpiler = Transpiler(initial_model=initial_model)
        transpiled_circuit = transpiler.transpile(qc, optimization_level=1)
        assert transpiled_circuit is not None
        transpiled_circuit.draw_circuit()
        # transpiled_circuit.plot_circuit()
        # plt.show()
