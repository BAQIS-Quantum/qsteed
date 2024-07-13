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

import multiprocessing
import os

import dill

from qsteed.passes.model import Model
from qsteed.passflow.passflow import PassFlow
from qsteed.transpiler.transpiler import Transpiler


def process_circuit(serialized_circuit, passflow, model):
    circuit = dill.loads(serialized_circuit)
    transpiler = Transpiler(passflow, model)
    compiled_circuit = transpiler.transpile(circuit)
    serialized_compiled_circuit = dill.dumps(compiled_circuit)
    return serialized_compiled_circuit


def parallel_process_circuits(circuits, passflows, models, num_processes=None):
    if isinstance(passflows, list):
        if len(passflows) != len(circuits):
            raise ValueError("The length of circuits and passflows are not equal.")
    elif isinstance(passflows, PassFlow):
        passflows = [passflows for _ in range(len(circuits))]
    else:
        raise ValueError(
            "Please enter the correct passflows data type, a single passflow or a list of multiple passflows.")

    if isinstance(models, list):
        if len(models) != len(circuits):
            raise ValueError("The length of circuits and models are not equal.")
    elif isinstance(models, Model):
        models = [models for _ in range(len(circuits))]
    else:
        raise ValueError(
            "Please enter the correct models data type, a single model or a list of multiple models.")

    serialized_circuits = [dill.dumps(circuit) for circuit in circuits]

    picked = zip(serialized_circuits, passflows, models)

    if num_processes is None:
        cpu_count = os.cpu_count()
        half_cpu_count = int(cpu_count / 2)  # Half full load
        if len(circuits) > half_cpu_count:
            num_processes = half_cpu_count
        else:
            num_processes = len(circuits)

    with multiprocessing.Pool(num_processes) as pool:
        results = pool.starmap(process_circuit, [(circuit, passflow, model) for circuit, passflow, model in picked])

    compiled_circuits = []
    for qc in results:
        compiled_circuit = dill.loads(qc)
        compiled_circuits.append(compiled_circuit)
    return compiled_circuits
