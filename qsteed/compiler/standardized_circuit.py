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
from qsteed.compiler.qasm_parser import qreg_creg


class StandardizedCircuit:
    """ Standardize quantum circuit (OpenQASM 2.0) """

    def __init__(self, circuit: str, rename: bool = True, rename_qreg: str = "q"):
        self.circuit = circuit
        self.qasm_lines = circuit.strip().splitlines()
        self.rename_qreg = rename_qreg
        self.rename = rename
        self.qreg_name, self.creg_name, self.qubit_num, self.cbit_num = qreg_creg(self.circuit)

    # def qreg_creg(self):
    #     self.qreg_name, self.creg_name, self.qubit_num, self.cbit_num = qreg_creg(self.circuit)

    def reset_barrier(self):
        """
        Reset barrier before measurement

        Args:
            circuit: quafu.QuantumCircuit

        Returns:
            circuit: quafu.QuantumCircuit of reset barrier
        """
        # circuit: qiskit QuantumCircuit

        measure_pattern = fr'measure\s+{self.qreg_name}\[(\d+)\]\s+->\s+{self.creg_name}\[(\d+)\];'
        measure_matches = re.findall(measure_pattern, self.circuit)
        measure_qlist = [int(match[0]) for match in measure_matches]
        measure_clist = [int(match[1]) for match in measure_matches]

        barrier_pattern = fr"barrier\s+((?:{self.qreg_name}\[\d+\],?\s*)+);"
        barrier_matches = re.findall(barrier_pattern, self.circuit)
        all_barrier_qubits = []
        for match in barrier_matches:
            qubit_indices = re.findall(fr'{self.qreg_name}\[(\d+)', match)
            barrier_qubits = [int(index) for index in qubit_indices]
            all_barrier_qubits.append(barrier_qubits)

        # qasm_lines = self.circuit.strip().splitlines()

        # Check if there is a barrier before the first measure.
        barrier_before_measure = False
        barrier_before_measure_lines = []
        for i, line in enumerate(self.qasm_lines):
            if 'measure' in line:
                if i > 0 and 'barrier' in self.qasm_lines[i - 1]:
                    barrier_before_measure = True
                    barrier_before_measure_lines.append(self.qasm_lines[i - 1])
                # break
        if 'barrier' in self.qasm_lines[-1]:
            barrier_before_measure_lines.append(self.qasm_lines[-1])

        # If there is no barrier before the measure, insert a barrier.
        if not barrier_before_measure:
            barrier_qubits = ','.join([fr"{self.qreg_name}[{q}]" for q in measure_qlist])
            barrier_instruction = fr"barrier {barrier_qubits};"
            first_measure_index = next(i for i, line in enumerate(self.qasm_lines) if 'measure' in line)
            self.qasm_lines.insert(first_measure_index, barrier_instruction)
        else:
            if len(barrier_before_measure_lines) == 1:
                qubit_indices = re.findall(fr'{self.qreg_name}\[(\d+)', barrier_before_measure_lines[0])
                barrier_qubits = set([int(index) for index in qubit_indices])
                # If the qubit sets of the measure and barrier are inconsistent,
                # delete the original barrier and insert a new barrier.
                if set(measure_qlist) != barrier_qubits:
                    last_index = len(self.qasm_lines) - 1 - self.qasm_lines[::-1].index(barrier_before_measure_lines[0])
                    self.qasm_lines.pop(last_index)
                    new_barrier_qubits = ','.join([f"{self.qreg_name}[{q}]" for q in measure_qlist])
                    new_barrier_instruction = f"barrier {new_barrier_qubits};"
                    for i, line in enumerate(self.qasm_lines):
                        if 'measure' in line:
                            self.qasm_lines.insert(i, new_barrier_instruction)
                            break
            else:
                for barrier in barrier_before_measure_lines:
                    last_index = len(self.qasm_lines) - 1 - self.qasm_lines[::-1].index(barrier)
                    self.qasm_lines.pop(last_index)

                new_barrier_qubits = ','.join([f"{self.qreg_name}[{q}]" for q in measure_qlist])
                new_barrier_instruction = f"barrier {new_barrier_qubits};"
                for i, line in enumerate(self.qasm_lines):
                    if 'measure' in line:
                        self.qasm_lines.insert(i, new_barrier_instruction)
                        break

        self.circuit = "\n".join(self.qasm_lines).strip()
        return self.circuit

    def standardized_circuit(self):
        """ Standardize quantum circuit.
        Args:
            circuit (OpenQASM 2.0):
        Returns:
            self.circuit (OpenQASM2.0): Standardized openqasm, adding measures and barriers at the end,
                                        and uniformly naming quantum registers as 'q'.
        """

        # self.qreg_creg()

        if self.qubit_num == 0:
            raise Exception("This is an empty circuit and cannot be executed.")

        if self.creg_name is None:
            # Insert creg after the qreg statement
            for i, line in enumerate(self.qasm_lines):
                if line.strip().startswith("qreg"):
                    self.creg_name = "meas"
                    self.cbit_num = self.qubit_num
                    self.qasm_lines.insert(i + 1, f"creg {self.creg_name}[{self.cbit_num}];")
                    break
            self.circuit = "\n".join(self.qasm_lines)
        elif self.creg_name is not None and self.cbit_num == 0:
            self.cbit_num = self.qubit_num
            self.circuit = self.circuit.replace(f"creg {self.creg_name}[0];", f"creg {self.creg_name}[{self.cbit_num}];")

        if 'measure' not in self.circuit:
            barrier = ','.join([f"{self.qreg_name}[{q}]" for q in range(self.qubit_num)])
            self.circuit = self.circuit + barrier
            measures = ''.join(
                [f"measure {self.qreg_name}[{i}] -> {self.creg_name}[{i}];\n" for i in range(self.qubit_num)])
            self.circuit = self.circuit + measures
        else:
            self.circuit = self.reset_barrier()

        if self.rename:
            self.circuit = self.circuit.replace(self.qreg_name + '[', f'{self.rename_qreg}[')
            self.qasm_lines = self.circuit.strip().splitlines()

        return self.circuit
