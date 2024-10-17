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


import configparser
import operator
import re
import time
from functools import reduce

from quafu import QuantumCircuit as quafuQC

from qsteed.backends.backend import Backend
from qsteed.compiler.program_verification import check_openqasm
from qsteed.compiler.qasm_parser import actually_bits, reset_qasm_bits, reset_real_qubits, get_measures, circuit_depth
from qsteed.compiler.qasm_parser import qreg_creg
from qsteed.compiler.standardized_circuit import StandardizedCircuit
from qsteed.config.get_config import get_config
from qsteed.graph.similar_substructure import similar_structure
from qsteed.passes.model import Model
from qsteed.passflow.passflow import PassFlow
from qsteed.resourcemanager.database_sql.database_query import query_vqpu, query_qpu, query_specified_vqpu, \
    generate_specified_vqpu
from qsteed.resourcemanager.database_sql.instantiating import get_qpu, get_vqpu, get_subqpu
from qsteed.transpiler.transpiler import Transpiler

QPUs = get_qpu()
VQPUs = get_vqpu()
SubQPUs = get_subqpu()

CONFIG_FILE = get_config()
CONFIG = configparser.ConfigParser()
CONFIG.read(CONFIG_FILE)

chips = CONFIG['Chips']
system_id_name = eval(CONFIG['Systems']['system_id_name'])
system_status = CONFIG['system_status']


class Compiler:
    def __init__(self,
                 circuit,
                 transpile: bool = True,
                 qpu_name: str = None,
                 qpu_id: int = None,
                 qubits_list: list = None,
                 optimization_level: int = 1,
                 passflow: PassFlow = None,
                 task_type: str = "qc",
                 repeat: int = 1,
                 vqpu_preferred: str = "fidelity",  # "structure"
                 vqpus: list = None,
                 ):
        self.circuit = circuit
        self.qpu_name = qpu_name
        self.qpu_id = qpu_id
        self.qubits_list = qubits_list
        self.optimization_level = optimization_level
        self.passflow = passflow
        self.vqpus = vqpus
        self.repeat = repeat
        self.task_type = task_type
        self.vqpu_preferred = vqpu_preferred
        self.transpile = transpile

        if self.qpu_name is None and self.qpu_id is not None:
            self.qpu_name = system_id_name[self.qpu_id]

        if self.qpu_name is None and self.qpu_id is None and self.transpile is False:
            raise ValueError("'transpile' is False, you need to specify the 'qpu_name' or 'qpu_id'.")

        if self.qubits_list is not None and self.qpu_name is None and self.qpu_id is None:
            raise ValueError("When specifying 'qubits_list', you must specify either 'qpu_name' or 'qpu_id'.")

    def compile(self):

        # Compile input_circuit
        compile_begin_time = time.time()

        # Determine the type of circuit
        if isinstance(self.circuit, quafuQC):
            self.circuit = self.circuit.to_openqasm(with_para=True)
        elif isinstance(self.circuit, str) and 'OPENQASM 2.0' in self.circuit:
            pass
        else:
            raise TypeError("The input_circuit needs to be quafu QuantumCircuit class or openQASM 2.0 string.")

        if self.transpile:
            transpiled_openqasm, used_vqpu, transpiled_circuit_depth, swap_count = self.call_transpiler(self.circuit)
            # Reset qubits to real physical qubits
            qpu = query_qpu(QPUs, qpu_name=used_vqpu.qpu_name)
            compiled_openqasm = reset_real_qubits(transpiled_openqasm, len(qpu[0].int_to_qubit), used_vqpu.vq_to_q)
        else:
            compiled_openqasm, used_vqpu, transpiled_circuit_depth, swap_count = self.call_untranspiler(self.circuit)
            q_to_vq = {q: vq for vq, q in used_vqpu.vq_to_q.items()}
            transpiled_openqasm = reset_real_qubits(compiled_openqasm, len(q_to_vq), q_to_vq)

            qpu = query_qpu(QPUs, qpu_name=used_vqpu.qpu_name)

        # Calculate compile time
        compile_time = time.time() - compile_begin_time

        # Check if openqasm satisfies the qubit coupling graph of the hardware
        # and check the number of single-qubit and two-qubit gates.
        check_qasm, single_nums, two_nums = check_openqasm(compiled_openqasm, qpu[0].structure,
                                                           len(qpu[0].int_to_qubit))
        try:
            int(check_qasm)
        except:
            print(check_qasm)

        # Get final_qubit2cbit and compiled_circuit_information
        measure_q2c = final_measure_mapping(compiled_openqasm)

        measure_qubits = sorted(measure_q2c, key=measure_q2c.get)
        compiled_circuit_information = {'transpiled_qasm': transpiled_openqasm,
                                        'qubits_mapping': used_vqpu.vq_to_q,
                                        'compiled_qasm': compiled_openqasm,
                                        'backend_name': used_vqpu.qpu_name,
                                        'basis_gates': used_vqpu.basis_gates,
                                        'number_of_single_gate': single_nums,
                                        'number_of_two_gate': two_nums,
                                        'compiled_circuit_depth': transpiled_circuit_depth,
                                        'number_of_qubits_used': len(measure_qubits),
                                        'hardware_qubits_for_measure': measure_qubits,
                                        'hardware_qubits_to_cbits': measure_q2c,
                                        'compile_time (s)': compile_time}
        return compiled_openqasm, measure_q2c, compiled_circuit_information

    def _sort_vqpus(self, sort_attribute: str = "coupling_list"):
        # TODO: Sort by QPU estimated free time, then by fidelity
        def calculate_product(vqpu):
            return reduce(operator.mul, (item[2] for item in getattr(vqpu, sort_attribute)), 1)

        return sorted(self.vqpus, key=calculate_product, reverse=True)

    def find_available_vqpus(self, qubits_num):
        # Finding available vqpus
        if self.qubits_list is None:
            available_vqpus = query_vqpu(VQPUs, qpu_name=self.qpu_name, qubits_num=qubits_num)
            if len(available_vqpus) == 0:
                raise ValueError("ERROR: No available VQPU found.")
        else:
            if self.qpu_name is None:
                raise ValueError("ERROR: If specifying a qubits list, it is necessary to also specify which backend.")
            else:
                available_vqpus = query_specified_vqpu(VQPUs, qpu_name=self.qpu_name, qubits_list=self.qubits_list)
                if len(available_vqpus) == 0:
                    available_vqpus = generate_specified_vqpu(QPUs, qpu_name=self.qpu_name,
                                                              qubits_list=self.qubits_list)

        return available_vqpus

    def get_optimal_vqpu(self, qubit_num: int = None):
        available_vqpus = self.find_available_vqpus(qubit_num)
        sorted_vqpus = _sort_vqpus(available_vqpus, sort_attribute='coupling_list')
        if self.vqpu_preferred == "fidelity":
            optimal_vqpu = sorted_vqpus[0]
        elif self.vqpu_preferred == "structure":
            keep_num = 10
            if len(sorted_vqpus) < keep_num:
                keep_num = len(sorted_vqpus)
            keep_vqpus = similar_structure(self.circuit, sorted_vqpus[0:keep_num])
            optimal_vqpu = keep_vqpus[0][0]
        else:
            raise ValueError("'vqpu_preferred' can only be 'fidelity' or 'structure'.")
        return optimal_vqpu

    def call_transpiler(self, circuit: str):
        # Calculate the physical and classical bits actually used by the circuit
        qubits, cbits = actually_bits(circuit)

        # Reset qubit/cbit
        if len(cbits) == 0:
            cbits = qubits
        input_qasm = reset_qasm_bits(circuit, qubits, cbits)

        # Standardized input circuit openqasm, adding measures and barriers at the end.
        new_circuit = StandardizedCircuit(input_qasm)
        input_qasm = new_circuit.standardized_circuit()

        # Finding available vqpus
        # available_vqpus = self.find_available_vqpus(len(qubits))

        if isinstance(input_qasm, quafuQC):
            logical_circuit = input_qasm
            qubit_num = logical_circuit.num
        elif isinstance(input_qasm, str) and 'OPENQASM 2.0' in input_qasm:
            qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(input_qasm)
            logical_circuit = quafuQC(qubit_num, cbit_num)
            logical_circuit.from_openqasm(input_qasm)
        else:
            raise TypeError("The input_circuit needs to be quafu QuantumCircuit class or openQASM 2.0 string.")

        used_vqpu = self.get_optimal_vqpu(qubit_num=qubit_num)
        initial_model = self._set_backend_model(used_vqpu)
        transpiler = Transpiler(initial_model=initial_model)
        transpiled_circuit = transpiler.transpile(logical_circuit, optimization_level=self.optimization_level)
        transpiled_openqasm = transpiled_circuit.to_openqasm(with_para=True)

        transpiled_circuit_depth = circuit_depth(transpiled_openqasm)

        swap_count = transpiler.model.datadict['add_swap_count']
        return transpiled_openqasm, used_vqpu, transpiled_circuit_depth, swap_count

    def call_untranspiler(self, circuit: str):
        if 'OPENQASM 2.0' not in circuit:
            raise TypeError("The circuit needs to be OpenQASM 2.0 string.")

        if self.qpu_name is None:
            raise ValueError("If not transpile, the qpu name must be given.")

        if "creg" not in circuit:
            raise Exception("The creg of the quantum circuit task is not given.")

        measures = get_measures(circuit)
        if len(measures) == 0:
            raise Exception("Without any measurements, the circuit cannot be executed.")

        # Calculate the physical and classical bits actually used by the circuit
        used_qubits, used_cbits = actually_bits(circuit)
        available_vqpus = self.find_available_vqpus(len(used_qubits))

        used_vqpu = None
        for vqpu in available_vqpus:
            if used_qubits == get_qubits_from_couplings(vqpu.coupling_list):
                used_vqpu = vqpu
                break
        if used_vqpu is None:
            used_vqpu = generate_specified_vqpu(QPUs, qpu_name=self.qpu_name, qubits_list=used_qubits)[0]

        # Standardized input circuit openqasm, adding measures and barriers at the end.
        new_circuit = StandardizedCircuit(circuit)
        compiled_openqasm = new_circuit.standardized_circuit()

        compiled_circuit_depth = circuit_depth(compiled_openqasm)
        swap_count = 0

        return compiled_openqasm, used_vqpu, compiled_circuit_depth, swap_count

    def _set_backend_model(self, vqpu):
        backend_properties = {
            'name': vqpu.vqpu_name,
            'backend_type': vqpu.backend_type,
            'qubits_num': vqpu.qubits_num,
            'coupling_list': vqpu.coupling_list,
            'basis_gates': eval(vqpu.basis_gates) if isinstance(vqpu.basis_gates, str) else vqpu.basis_gates,
        }
        backend_instance = Backend(**backend_properties)
        initial_model = Model(backend=backend_instance)
        return initial_model


def _sort_vqpus(vqpus, sort_attribute=None):
    def calculate_product(vqpu):
        return reduce(operator.mul, (item[2] for item in getattr(vqpu, sort_attribute)), 1)

    return sorted(vqpus, key=calculate_product, reverse=True)


def _set_backend_model(vqpu):
    backend_properties = {
        'name': vqpu.vqpu_name,
        'backend_type': vqpu.backend_type,
        'qubits_num': vqpu.qubits_num,
        'coupling_list': vqpu.coupling_list,
        'basis_gates': vqpu.basis_gates,
    }
    backend_instance = Backend(**backend_properties)
    initial_model = Model(backend=backend_instance)
    return initial_model


def final_measure_mapping(compiled_openqasm):
    """
    Get measurements from final compiled QASM.

    Args:
        compiled_openqasm(string): OpenQASM 2.0

    Returns:
        final_measure_q2c(dict): {physics_bit: classical_bit, ...}

    """
    if 'OPENQASM' not in compiled_openqasm:
        print('Warning: need openqasm string! Check if openqasm headers are included!')
    if 'creg' not in compiled_openqasm:
        print('Warning: The circuit has no classic registers!')
    if 'measure' not in compiled_openqasm:
        print('Warning: The circuit has no measure!')
    qasm_str_list = compiled_openqasm.replace('\n', '').split(';')
    measure_str_list = []
    for elem in qasm_str_list:
        if 'measure' in elem:
            measure_str_list.append(elem)

    final_measure_q2c = {}
    for measure in measure_str_list:
        measure_mapping = re.findall(r"\d+\.?\d*", measure)
        measure_mapping = [int(bit) for bit in measure_mapping]
        final_measure_q2c.update({measure_mapping[0]: measure_mapping[1]})
    return final_measure_q2c


def get_qubits_from_couplings(coupling_list):
    """
    Get unique qubits from a coupling list.
    Args:
        coupling_list (list): List of couplings, where each element is a list [q1, q2, value].
    Returns:
        list: Unique qubit indices.
    """
    unique_qubits = set()
    for coupling in coupling_list:
        unique_qubits.update(coupling[:2])
    return sorted(list(unique_qubits))
