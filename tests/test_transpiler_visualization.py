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

"""Test the TranspilerVis class"""

from qsteed.transpiler.transpiler_visualization import TranspilerVis, dynamic_draw, draw_allpass_circuits
from .shared_utils import get_random_circuit, get_passflow, get_initial_model


class TestTranspilerVis:
    """Test cases for the TranspilerVis class and related visualization functionalities."""
    def test_transpiler_vis(self):
        """Test the transpilation visualization process."""
        qc = get_random_circuit()
        passflow = get_passflow()
        initial_model = get_initial_model()

        transpiler_vis = TranspilerVis(passflow, initial_model)
        transpiled_circuit, info, short_info = transpiler_vis.transpile_vis(qc)

        assert transpiled_circuit is not None

        print("The information about the input index pass, here the input index is dynamic")
        # dynamic_draw: Display intermediate compilation results interactively. Please uncomment when in use.
        # dynamic_draw(info, short_info)
        draw_allpass_circuits(info)
