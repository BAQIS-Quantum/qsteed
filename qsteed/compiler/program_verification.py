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


import re
import warnings
from qsteed.compiler.qasm_parser import qreg_creg


def check_openqasm(qasm, coupling_list, chip_qubit_num):
    """ Compile the input openqasm into quafu hardware executable openqasm.
    Args:
        qasm: openqasm 2.0 string
        coupling_list: the qubit coupling graph of the hardware: [[0,1],[1,0],[1,2],...]
        chip_qubit_num:
    Returns:
        bool: True/False
        single_nums: single-qubit gate counts
        two_nums: two-qubit gate counts
    """
    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(qasm)
    if 'OPENQASM' not in qasm:
        warnings.warn("Need openqasm string! Check if openqasm headers are included!")
    qasm_str_list = qasm.strip().splitlines()
    gate_str_list = []
    measure_str_list = []
    header_str_list = []
    for elem in qasm_str_list:
        if 'measure' in elem or 'barrier' in elem:
            measure_str_list.append(elem)
        elif 'OPENQASM' in elem or 'include' in elem:
            header_str_list.append(elem)
        elif 'qreg' in elem or 'creg' in elem:
            header_str_list.append(elem)
            required_qubits = int(re.findall(r'(\d+)', elem)[0])
            # Check if the number of required qubits exceeds the number of chip qubits.
            if required_qubits > chip_qubit_num:
                raise Exception("The required number of qubits is %s, which exceeds the number of " \
                                "chip qubits by %s." % (required_qubits, chip_qubit_num))
        else:
            gate_str_list.append(elem)

    # Build the legal matrix according to the coupling_list
    legal_matrix = [[False for _ in range(chip_qubit_num)] for _ in range(chip_qubit_num)]
    for coupling in coupling_list:
        legal_matrix[coupling[0]][coupling[1]] = True

    single_nums = 0
    two_nums = 0
    for line in gate_str_list:
        gate_match = re.match(fr'(\w+)\s*(\(.*?\))?\s+{qreg_name}\[(\d+)\](?:,{qreg_name}\[(\d+)\])?;', line)
        if gate_match:
            qubits = [int(q) for q in gate_match.groups()[2:] if q is not None]
            if len(qubits) == 1:
                single_nums += 1
                if qubits[0] > chip_qubit_num:
                    raise Exception(f"{line} exceeds system qubits number %s" % chip_qubit_num)
            elif len(qubits) == 2:
                # Check if openqasm satisfies the qubit coupling graph of the hardware
                two_nums += 1
                if legal_matrix[qubits[0]][qubits[1]]:
                    continue
                else:
                    raise ValueError(f"Error: illegal gate '{line}'" +
                                     f", qubits {qubits[0]} and {qubits[1]} are not directly coupled.")
            else:
                raise ValueError(f"Error: illegal gate '{line}'" +
                                 ", quantum gate exceeding 2-qubits are not supported.")

    # Check the number of single-qubit and two-qubit gates
    if two_nums > 100000:
        return "Error: The number of two-qubit gates cannot exceed 100000!" \
               + " The compiled circuit contains " + str(two_nums) + " two-qubit gates.", single_nums, two_nums
    elif single_nums > 5000000:
        return "Error: The number of single-qubit gates cannot exceed 5000000!" \
               + " The compiled circuit contains " + str(single_nums) + " single qubit gates.", single_nums, two_nums
    elif single_nums == 0 and two_nums == 0:
        return "Warning: this is an empty circuit!", single_nums, two_nums

    return True, single_nums, two_nums
