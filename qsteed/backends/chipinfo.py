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

from qsteed.config.get_config import get_config

CONFIG_FILE = get_config()
CONFIG = configparser.ConfigParser()
CONFIG.read(CONFIG_FILE)

chips = CONFIG['Chips']
system_id_name = eval(CONFIG['Systems']['system_id_name'])
system_status = CONFIG['system_status']


class ChipInfo:
    def __init__(self, name, chip_info_dict=None):
        self.name = name
        self.system_id = None
        self.system_status = None
        self.qubit_num = None
        self.calibration_time = None
        self.all_topological_structure = None
        self.topological_structure = None
        self.all_qubits_info = None
        self.qubits_info = None
        self.basis_gates = None
        self.priority_qubits = None
        self.chip_info_dict = chip_info_dict
        self.int_to_qubit = None
        self.qubit_to_int = None

    def initialize_chip(self):
        chip = chips.get(self.name.lower())

        if chip:
            chip = eval(chip)
            self.system_id = chip['system_id']
            self.name = chip['name']
            self.set_chip_dict()
            if sorted(self.basis_gates) != sorted(chip['basis_gates']):
                print("Warning: The gate sets given in the configuration file and the chip information "
                      "file are inconsistent. The configuration file gate set is selected by default.")
            self.basis_gates = chip['basis_gates']
            if 'id' not in self.basis_gates:
                self.basis_gates.append('id')
            self.qubit_num = chip['qubit_num']
        else:
            raise NameError(f"Could not find system name mapping for {self.name}")

    def set_chip_dict(self):
        if self.chip_info_dict is not None:
            if self.system_id != self.chip_info_dict['system_id']:
                raise ValueError(
                    'System id and name do not match, please check given chip_info_dict or system_id_name')
            self.calibration_time = self.chip_info_dict.get('calibration_time', None)
            self.all_topological_structure = self.chip_info_dict.get('all_topological_structure', None)
            self.topological_structure = self.chip_info_dict.get('topological_structure', None)
            self.all_qubits_info = self.chip_info_dict.get('all_qubits_info', None)
            self.qubits_info = self.chip_info_dict.get('qubits_info', None)
            self.basis_gates = self.chip_info_dict.get('basis_gates', None)
            self.priority_qubits = self.chip_info_dict.get('priority_qubits', "[]")
            self.int_to_qubit = self.chip_info_dict.get('int_to_qubit', None)
            self.qubit_to_int = self.chip_info_dict.get('qubit_to_int', None)
        else:
            raise ValueError('The chip_info_dict is not fount.')
