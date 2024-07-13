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

from quafu.elements.element_gates import CXGate, CZGate, ISwapGate, CPGate

from qsteed.passes.unroll import rules
from qsteed.passes.unroll.rules import *

class_names = rules.__all__

# Use globals() to get class objects and create a dictionary
Rules_dict = {}
# CX_rules = defaultdict(list)
for class_name in class_names:
    class_obj = globals().get(class_name)
    if class_obj is not None:
        if class_obj().original != CXGate.name.lower():
            Rules_dict[class_obj().original] = [class_obj().basis, class_obj()]
        # else:
        #     CX_rules[class_obj().original].append([class_obj().basis, class_obj()])

# Rules_dict = {SwapToCNOT().original: [SwapToCNOT().basis, SwapToCNOT()],
#               RZZToCNOT().original: [RZZToCNOT().basis, RZZToCNOT()]}

# TODO: Conversion of CNOT with other two-qubit gates, such as iswap, cz
CX_rules = {CZGate.name.lower(): {CNOTToCZ().original: [CNOTToCZ().basis, CNOTToCZ()]},
            ISwapGate.name.lower(): {CNOTToISWAP().original: [CNOTToISWAP().basis, CNOTToISWAP()]},
            CPGate(0, 1, 0).name.lower(): {CNOTToCP().original: [CNOTToCP().basis, CNOTToCP()]}}
