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
from collections import defaultdict

# from numpy import pi


def qreg_creg(circuit: str):
    """
    Extract quantum and classical register names and sizes from the OpenQASM 2.0 circuit.

    Args:
        circuit (str): OpenQASM 2.0 string

    Returns:
        qreg_name (str): quantum register name
        creg_name (str): classical register name
        qubit_num (int): quantum register size
        cbit_num (int): classical register size
    """
    if isinstance(circuit, str) and 'OPENQASM 2.0' in circuit and 'include "qelib1.inc"' in circuit:
        qreg_pattern = r'qreg\s+(\w+)\[\d+\];'
        qubits_pattern = r'qreg\s+\w+\[(\d+)\];'
        creg_pattern = r'creg\s+(\w+)\[\d+\];'
        cbits_pattern = r'creg\s+\w+\[(\d+)\];'

        qreg_name = re.search(qreg_pattern, circuit)
        creg_name = re.search(creg_pattern, circuit)
        qubits = re.search(qubits_pattern, circuit)
        cbits = re.search(cbits_pattern, circuit)

        qreg_name = qreg_name.group(1) if qreg_name else None
        creg_name = creg_name.group(1) if creg_name else None
        qubit_num = int(qubits.group(1)) if qubits else 0
        cbit_num = int(cbits.group(1)) if cbits else 0
    else:
        raise TypeError("Please input a circuit in OpenQASM 2.0 format.")

    measured_names = re.findall(fr'measure\s+{qreg_name}\[\d+]\s+->\s+(\w+)\[\d+]', circuit)

    if measured_names:
        measured_name = measured_names[0]
        if measured_name != creg_name:
            raise NameError("The measurement name do not correspond to the defined classic register name.")

    return qreg_name, creg_name, qubit_num, cbit_num


def actually_bits(input_qasm: str):
    """
    Calculate the physical and classical bits actually used by the circuit.

    Args:
        input_qasm (str): OpenQASM 2.0 string.

    Returns:
        qubits (list): List of physical qubits used.
        cbits (list): List of classical bits used.
    """
    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(input_qasm)

    filtered_qasm = re.sub(r'^.*(qreg|creg|barrier).*$\n?', '', input_qasm, flags=re.MULTILINE)

    # Extracting qubits
    qubit_matches = re.findall(fr'(?<!qreg\s)(?<!barrier\s){qreg_name}\[(\d+)]', filtered_qasm)
    qubits = sorted(map(int, set(qubit_matches)))

    # Extracting cbits
    cbit_matches = re.findall(fr'->\s+{creg_name}\[(\d+)]', filtered_qasm)
    cbits = sorted(map(int, set(cbit_matches)))

    return qubits, cbits


def insert_creg(qasm: str, cbits: int, creg_name: str = None) -> str:
    """
    Insert a classical register (creg) definition after the quantum register (qreg) definition.

    Args:
        qasm (str): OpenQASM 2.0 code as a string.
        cbits (int): Number of classical bits to define.

    Returns:
        str: Modified OpenQASM 2.0 code with creg added.
    """
    # Define the regex pattern for matching qreg definitions
    qreg_pattern = fr'(qreg\s+\w+\[\d+\];)'

    # Replacement function to insert creg after qreg
    if creg_name is None:
        creg_name = "c"

    def replacement(match):
        return fr'{match.group(0)}\ncreg\s+{creg_name}\[{cbits}];'

    # Perform the substitution
    return re.sub(qreg_pattern, replacement, qasm)


def reset_qasm_bits(input_qasm: str, qubits: list, cbits: list):
    """
    Reset qubit/cbit

    Args:
        input_qasm(string): OpenQASM 2.0 string
        qubits(list):
        cbits(list):

    Returns:
        input_qasm(string): OpenQASM 2.0 string
    """
    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(input_qasm)
    used_qubit_num = len(qubits)
    used_cbit_num = len(cbits)
    input_qasm = re.sub(fr'qreg\s+{qreg_name}\[{qubit_num}]', fr'qreg usedqubits[{used_qubit_num}]', input_qasm)

    if 'creg' in input_qasm:
        if used_cbit_num == 0:
            input_qasm = re.sub(fr'creg\s+{creg_name}\[{cbit_num}];', '', input_qasm)
        else:
            input_qasm = re.sub(fr'creg\s+{creg_name}\[{cbit_num}]', fr'creg usedcbits[{used_cbit_num}]', input_qasm)

    else:
        input_qasm = insert_creg(input_qasm, len(cbits), creg_name)

    lq_to_q = {q: qubits[q] for q in range(len(qubits))}
    lc_to_c = {c: cbits[c] for c in range(len(cbits))}

    for lq, q in sorted(lq_to_q.items(), key=lambda item: item[1], reverse=True):
        input_qasm = re.sub(fr'{qreg_name}\[{q}]', fr'qreset[{lq}]', input_qasm)
    for lc, c in sorted(lc_to_c.items(), key=lambda item: item[1], reverse=True):
        input_qasm = re.sub(fr'->\s+{creg_name}\[{c}]', fr'-> creset[{lc}]', input_qasm)

    input_qasm = re.sub(fr'->\s+creset\[', fr'-> {creg_name}[', input_qasm)
    input_qasm = re.sub(fr'qreset\[', fr'{qreg_name}[', input_qasm)
    input_qasm = re.sub(fr'qreg\s+usedqubits\[', fr'qreg {qreg_name}[', input_qasm)
    input_qasm = re.sub(fr'creg\s+usedcbits\[', fr'creg {creg_name}[', input_qasm)

    return input_qasm


def reset_real_qubits(compiled_openqasm: str, physical_qubits: int, req_to_q: dict):
    """
    Reset qubits to real physical qubits.

    Args:
        compiled_openqasm(str): openqasm string
        physical_qubits(int): The number of qubits in quantum chip.
        req_to_q(dict): logical qubit to physical qubit, example {0: 108, 1: 109, 2: 119}

    Returns:
        compiled_openqasm(str): openqasm string
    """
    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(compiled_openqasm)

    compiled_openqasm = re.sub(fr'qreg\s+{qreg_name}\[{qubit_num}]', fr'qreg {qreg_name}[{physical_qubits}]',
                               compiled_openqasm)

    def replace_match(match):
        qubit_index = int(match.group(1))
        return fr"{qreg_name}[{req_to_q.get(qubit_index, qubit_index)}]"

    qreg_pattern = fr'qreg\s+{qreg_name}'
    qreg_lines = re.findall(qreg_pattern, compiled_openqasm)

    def replace_gate_lines(qasm):
        lines = qasm.splitlines()
        processed_lines = []
        for line in lines:
            if any(decl in line for decl in qreg_lines):
                processed_lines.append(line)
            else:
                line = re.sub(fr'{qreg_name}\[(\d+)]', replace_match, line)
                processed_lines.append(line)
        return '\n'.join(processed_lines)

    compiled_openqasm = replace_gate_lines(compiled_openqasm)
    compiled_openqasm = re.sub(fr"delay\((\d+)dt\)", fr"delay(\1ns)", compiled_openqasm)

    return compiled_openqasm


def reorder(qasm):
    """
    Align the QASM circuit to the left.

    Args:
        qasm: OpenQASM 2.0

    Returns:
        reordered_str: OpenQASM 2.0
    """
    if 'OPENQASM 2.0' not in qasm or 'include "qelib1.inc"' not in qasm:
        raise ValueError('Error: OpenQASM header is missing!')
    if 'qreg' not in qasm:
        raise ValueError('Error: The circuit has no quantum registers!')
    if 'creg' not in qasm:
        print('Warning: The circuit has no classic registers!')
    if 'measure' not in qasm:
        print('Warning: The circuit has no measure!')

    # qasm_str_list = qasm.replace('\n', '').split(';')
    qasm_str_list = qasm.strip().splitlines()
    gate_str_list = []
    measure_str_list = []
    header_str_list = []
    for elem in qasm_str_list:
        if 'measure' in elem or 'barrier' in elem:
            measure_str_list.append(elem)
        elif 'OPENQASM' in elem or 'include' in elem or 'qreg' in elem or 'creg' in elem:
            header_str_list.append(elem)
        else:
            gate_str_list.append(elem)

    num_qubit = int(re.findall(r"\d+\.?\d*", qasm.split('qreg')[1].split(';')[0])[0])
    depth_qubit = [0] * num_qubit
    gate_list = []
    for gate_str in gate_str_list:
        if '[' in gate_str:
            gate = gate_str.split()
            gate_qubits = re.findall(r"\d+\.?\d*", gate[1])
            gate_qubits = [int(qubit) for qubit in gate_qubits]
            if (len(gate_qubits) == 1):
                qubit = gate_qubits[0]
                depth_gate = depth_qubit[qubit] + 1
                depth_qubit[qubit] = depth_gate
                gate_list.append([gate_str, depth_gate])
            else:
                qubit1 = gate_qubits[0]
                qubit2 = gate_qubits[1]
                depth_gate = max(depth_qubit[qubit1], depth_qubit[qubit2]) + 1
                depth_qubit[qubit1] = depth_gate
                depth_qubit[qubit2] = depth_gate
                gate_list.append([gate_str, depth_gate])
    gate_list.sort(key=lambda x: x[1])
    openqasm_header = ';\n'.join(header_str_list)
    openqasm_measure = ';\n'.join(measure_str_list)
    gate_list_without_depth = [gate[0] for gate in gate_list]
    reordered_str = ';\n'.join(gate_list_without_depth)
    reordered_str = openqasm_header + ';\n' + reordered_str + ';\n' + openqasm_measure + ';\n'
    return reordered_str


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


def get_measures(qasm):
    """
    Get the measurement mapping from QASM code.
    Args:
        qasm (str): The QASM code.
    Returns:
        dict: Mapping of quantum bits to classical bits.
    """
    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(qasm)
    measure_str_list = [line for line in qasm.splitlines() if 'measure' in line]
    measure_mapping = {
        int(match[0]): int(match[1])
        for line in measure_str_list
        for match in re.findall(fr'{qreg_name}\[(\d+)].*{creg_name}\[(\d+)]', line)
    }
    return measure_mapping


def circuit_depth(qasm: str):
    """Calculating quantum circuit depth
    Args:
        qasm (str): The QASM code.
    Returns:
        depth: quantum circuit depth
    """
    # Match all operations
    pattern = r'(\w+)(?:\([^\)]*\))?\s+((?:\w+\[\d+\](?:,\s*)?)*)\s*(?:->\s*\w+\[\d+\])?;'
    matches = re.findall(pattern, qasm)

    qubit_usage = defaultdict(int)
    # Iterate over all matching gate operations
    for gate, bits in matches:
        if gate not in ['qreg', 'creg']:
            bit_list = re.findall(r'\w+\[(\d+)]', bits)
            bit_indices = [int(b) for b in bit_list]
            # Get the current maximum depth of bits involved
            current_depth = max([qubit_usage[bit] for bit in bit_indices], default=0)
            new_depth = current_depth + 1
            if gate == 'barrier':
                # barrier operation: synchronize the depth of all bits
                for bit in bit_indices:
                    qubit_usage[bit] = new_depth
            else:
                for bit in bit_indices:
                    qubit_usage[bit] = max(qubit_usage[bit], new_depth)

    depth = max(qubit_usage.values(), default=0)

    return depth


# def to_openqasm(qc):
#     """
#     Transform quantum circuits(quafu.QuantumCircuit) into strings(modified OpenQASM 2.0).
#
#     Args:
#         qc: quafu.QuantumCircuit
#
#     Returns:
#         qasm(string): modified OpenQASM 2.0
#     """
#     ops = qc.data
#     num = qc.qregs[0].size
#     if qc.cregs:
#         cnum = qc.cregs[0].size
#     qasm = '''OPENQASM 2.0;\ninclude "qelib1.inc";\n'''
#     qasm += "qreg q[%d];\n" % num
#     if qc.cregs:
#         qasm += "creg meas[%d];\n" % cnum
#     for op in ops:
#         name = op[0].name
#         paras = op[0].params
#         inds = [qb.index for qb in op[1]]
#         if name == "delay":
#             qasm += "delay(%d%s) q[%d];\n" % (op[0].duration, op[0].unit, inds[0])
#         elif name == "measure":
#             cbits = [cb.index for cb in op[2]]
#             qasm += "measure q[%d] -> meas[%d];\n" % (inds[0], cbits[0])
#         elif not paras:  # Fixed
#             qasm += "%s " % name + ",".join(["q[%d]" % p for p in inds]) + ";\n"
#         else:
#             qasm += "%s" % name + "(" + ",".join(["%s" % p for p in paras]) + ") " + ",".join(
#                 ["q[%d]" % p for p in inds]) + ";\n"
#     return qasm


# def to_openqasm(qc) -> str:
#     """
#     Convert the circuit to openqasm text.
#
#     Args:
#         qc: quafu.QuantumCircuit
#
#     Returns:
#         qasm(string): modified OpenQASM 2.0
#     """
#     qasm = "OPENQASM 2.0;\ninclude \"qelib1.inc\";\n"
#     qasm += "qreg q[%d];\n" % qc.num
#     qasm += "creg meas[%d];\n" % len(qc.measures)
#     for gate in qc.gates:
#         qasm += gate.to_qasm() + ";\n"
#
#     for key in qc.measures:
#         qasm += "measure q[%d] -> meas[%d];\n" % (key, qc.measures[key])
#
#     qc.openqasm = qasm
#     return qasm


# def to_openqasm(qc) -> str:
#     """
#     Convert the circuit to openqasm text.
#
#     Args:
#         qc: quafu.QuantumCircuit
#
#     Returns:
#         qasm(string): modified OpenQASM 2.0
#     """
#     # qc.to_openqasm()
#     return qc.to_openqasm()


# def from_openqasm(openqasm: str) -> QuantumCircuit:
#     """
#     Transform strings(modified OpenQASM 2.0) into quantum circuits(quabc.QuantumCircuit).
#
#     Args:
#         openqasm(string): modified OpenQASM 2.0
#
#     Returns:
#         temp_qc: quafu.QuantumCircuit
#     """
#     # self.openqasm = openqasm
#     # lines = self.openqasm.strip("\n").splitlines(";")
#     lines = openqasm.splitlines()
#     lines = [line for line in lines if line]
#     measures = {}
#     measured_qubits = []
#     global_valid = True
#     num = int(re.findall(r"\d+\.?\d*", openqasm.split('qreg')[1].split(';')[0])[0])
#     cnum = 0
#     if 'creg' in openqasm:
#         cnum = int(re.findall(r"\d+\.?\d*", openqasm.split('creg')[1].split(';')[0])[0])
#     temp_qc = QuantumCircuit(num, cnum)
#     for line in lines[2:]:
#         if line:
#             operations_qbs = line.split(" ", 1)
#             operations = operations_qbs[0]
#             if operations == "qreg":
#                 pass
#             elif operations == "creg":
#                 pass
#             elif operations == "measure":
#                 qbs = operations_qbs[1]
#                 indstr = re.findall(r"\d+", qbs)
#                 inds = [int(indst) for indst in indstr]
#                 mb = inds[0]
#                 cb = inds[1]
#                 temp_qc.measure([mb], [cb])
#                 measured_qubits.append(mb)
#             else:
#                 qbs = operations_qbs[1]
#                 indstr = re.findall(r"\d+", qbs)
#                 inds = [int(indst) for indst in indstr]
#                 valid = True
#                 for pos in inds:
#                     if pos in measured_qubits:
#                         valid = False
#                         global_valid = False
#                         break
#
#                 if valid:
#                     if operations == "barrier":
#                         temp_qc.barrier(inds)
#
#                     else:
#                         sp_op = operations.split("(")
#                         gatename = sp_op[0]
#                         if gatename == "delay":
#                             paras = sp_op[1].strip("()")
#                             duration = int(re.findall(r"\d+", paras)[0])
#                             unit = re.findall("[a-z]+", paras)[0]
#                             temp_qc.delay(inds[0], duration, unit)
#                         elif gatename == "xy":
#                             paras = sp_op[1].strip("()")
#                             duration = int(re.findall(r"\d+", paras)[0])
#                             unit = re.findall("[a-z]+", paras)[0]
#                             temp_qc.xy(min(inds), max(inds), duration, unit)
#                         else:
#                             if len(sp_op) > 1:
#                                 paras = sp_op[1].strip("()")
#                                 parastr = paras.split(",")
#                                 paras = [eval(parai, {"pi": pi}) for parai in parastr]
#
#                             if gatename == "cx":
#                                 temp_qc.cnot(inds[0], inds[1])
#                             elif gatename == "cy":
#                                 temp_qc.cy(inds[0], inds[1])
#                             elif gatename == "cz":
#                                 temp_qc.cz(inds[0], inds[1])
#                             elif gatename == "cs":
#                                 temp_qc.cs(inds[0], inds[1])
#                             elif gatename == "ct":
#                                 temp_qc.ct(inds[0], inds[1])
#                             elif gatename == "cp":
#                                 temp_qc.cp(inds[0], inds[1], paras[0])
#                             elif gatename == "swap":
#                                 temp_qc.swap(inds[0], inds[1])
#                             elif gatename == "iswap":
#                                 temp_qc.iswap(inds[0], inds[1])
#                             elif gatename == "rx":
#                                 temp_qc.rx(inds[0], paras[0])
#                             elif gatename == "ry":
#                                 temp_qc.ry(inds[0], paras[0])
#                             elif gatename == "rz":
#                                 temp_qc.rz(inds[0], paras[0])
#                             elif gatename == "p":
#                                 temp_qc.p(inds[0], paras[0])
#                             elif gatename == "x":
#                                 temp_qc.x(inds[0])
#                             elif gatename == "y":
#                                 temp_qc.y(inds[0])
#                             elif gatename == "z":
#                                 temp_qc.z(inds[0])
#                             elif gatename == "h":
#                                 temp_qc.h(inds[0])
#                             elif gatename == "id":
#                                 temp_qc.id(inds[0])
#                             elif gatename == "w":
#                                 temp_qc.w(inds[0])
#                             elif gatename == "sw":
#                                 temp_qc.sw(inds[0])
#                             elif gatename == "s":
#                                 temp_qc.s(inds[0])
#                             elif gatename == "sdg":
#                                 temp_qc.sdg(inds[0])
#                             elif gatename == "t":
#                                 temp_qc.t(inds[0])
#                             elif gatename == "tdg":
#                                 temp_qc.tdg(inds[0])
#                             elif gatename == "sx":
#                                 temp_qc.sx(inds[0])
#                             elif gatename == "sxdg":
#                                 temp_qc.sxdg(inds[0])
#                             elif gatename == "sy":
#                                 temp_qc.sy(inds[0])
#                             elif gatename == "sydg":
#                                 temp_qc.sydg(inds[0])
#                             elif gatename == "ccx":
#                                 temp_qc.toffoli(inds[0], inds[1], inds[2])
#                             elif gatename == "cswap":
#                                 temp_qc.fredkin(inds[0], inds[1], inds[2])
#                             elif gatename == "u1":
#                                 temp_qc.rz(inds[0], paras[0])
#                             elif gatename == "u2":
#                                 temp_qc.rz(inds[0], paras[1])
#                                 temp_qc.ry(inds[0], pi / 2)
#                                 temp_qc.rz(inds[0], paras[0])
#                             elif gatename == "u3":
#                                 temp_qc.rz(inds[0], paras[2])
#                                 temp_qc.ry(inds[0], paras[0])
#                                 temp_qc.rz(inds[0], paras[1])
#                             elif gatename == "rxx":
#                                 temp_qc.rxx(inds[0], inds[1], paras[0])
#                             elif gatename == "ryy":
#                                 temp_qc.ryy(inds[0], inds[1], paras[0])
#                             elif gatename == "rzz":
#                                 temp_qc.rzz(inds[0], inds[1], paras[0])
#                             elif gatename == "mcx":
#                                 temp_qc.mcx(inds[0:len(inds) - 1], inds[-1])
#                             elif gatename == "mcy":
#                                 temp_qc.mcy(inds[0:len(inds) - 1], inds[-1])
#                             elif gatename == "mcz":
#                                 temp_qc.mcz(inds[0:len(inds) - 1], inds[-1])
#                             else:
#                                 raise NameError("Operations %s may be not supported by quafu QuantumCircuit "
#                                                 "class currently." % gatename)
#                                 # print("Warning: Operations %s may be not supported by "
#                                 #       "QuantumCircuit class currently." % gatename)
#     if measures:
#         temp_qc.measure(list(measures.keys()), list(measures.values()))
#     if not global_valid:
#         print("Warning: All operations after measurement will be removed for executing on experiment")
#     return temp_qc