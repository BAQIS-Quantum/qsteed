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


from qsteed.compiler.compiler import Compiler
from qsteed.passflow.passflow import PassFlow


def call_compiler_api(circuit: str = None,
                      transpile: bool = True,
                      qpu_name: str = None,
                      qpu_id: int = None,
                      qubits_list: list = None,
                      optimization_level: int = 1,
                      passflow: PassFlow = None,
                      task_type: str = "qc",
                      repeat: int = 1,
                      vqpu_preferred: str = "fidelity",  # "structure"
                      **task_info,
                      ):
    """

    Args:
        circuit: OpenQASM 2.0
        transpile (bool): True - Perform quantum circuit transpilation optimization.
                   False - No transpilation is performed, only circuit legitimacy verification is performed.
                           The qpu_name must be specified in this case.
        qpu_name (str): chip's name. If not specified, select from all available computing resources.
        qpu_id (id): chip's ID. qpu_id and qpu_name correspond one-to-one.
        qubits_list (list): A list of real physical qubits used by the specified task. e.g. [3,4,5,6]
                    When specifying 'qubits_list', you must specify either 'qpu_name' or 'qpu_id'.
        optimization_level (int):  0: Only quantum gates decomposition
                             1: Distance-based sabre algorithm (qubit mapping and routing)
                                and simple gate optimization (merging and elimination)
                             2: Fidelity-based sabre algorithm (qubit mapping and routing)
                                and simple gate optimization (merging and elimination)
                             3: Hybrid sabre algorithm (qubit mapping and routing)
                                and simple gate optimization (merging and elimination)
        passflow (PassFlow): Customizing the transpilation process
        task_type (str): "qc" - quantum circuit task
                         "vqa" - Variational quantum algorithm task (subsequent versions will provide)
        repeat (int): Transpilation is repeated several times,
                      from which the best transpilation result is selected (subsequent versions will provide).
        vqpu_preferred (str): "fidelity": Choose the VQPU with the highest fidelity.
                              "structure": Choose the VQPU whose qubits coupling structure best matches the task.
        task_info (dict): The above parameters can be packaged

    Returns:
        compiled_info (list): [compiled_openqasm, measure_q2c, compiled_circuit_information]
                                compiled_openqasm (str): Compiled quantum circuit (OpenQASM 2.0),
                                measure_q2c (dict): Mapping of physical qubits to classical bits,
                                compiled_circuit_information (dict): Compilation result information
    """

    # Get task parameters
    circuit = task_info.get("circuit", circuit)
    transpile = task_info.get("transpile", transpile)
    qpu_name = task_info.get("qpu_name", qpu_name)
    qpu_id = task_info.get("qpu_id", qpu_id)
    qubits_list = task_info.get("qubits_list", qubits_list)
    optimization_level = task_info.get("optimization_level", optimization_level)
    passflow = task_info.get("passflow", passflow)
    task_type = task_info.get("task_type", task_type)
    repeat = task_info.get("repeat", repeat)
    vqpu_preferred = task_info.get("vqpu_preferred", vqpu_preferred)

    compiler = Compiler(circuit, transpile, qpu_name, qpu_id, qubits_list, optimization_level,
                        passflow, task_type, repeat, vqpu_preferred)
    compiled_openqasm, measure_q2c, compiled_circuit_information = compiler.compile()
    compiled_info = [compiled_openqasm, measure_q2c, compiled_circuit_information]
    return compiled_info
