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


import json
import os
from qsteed.apis.resourceDB_api import transform_quafu_chip, update_chip_api
from quark import Task
from qsteed.apis.compiler_api import call_compiler_api


TOKEN = 'JeRkiDCGU57LD5CPHjj:JF`pj:fmpw5GIO8twWfNsX5/:K{N4dUNyRkM1FUPxZ{O{h{OypkJxiY[jxjJ1NkP5BkP4FEJyFUM3BUM3JENzJjPjRYZqKDMypkJtWnemynJtJjck6zZi6zdqGYZjClfpWIfjpkJzW3d2Kzf'


def test_send_quafu_task():
    tmgr = Task(token=TOKEN)

    qasm = """
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[4];
    creg meas[4];
    h q[0];
    cx q[0],q[1];
    cx q[1],q[2];
    cx q[2],q[3];
    measure q[0] -> meas[0];
    measure q[1] -> meas[1];
    measure q[2] -> meas[2];
    measure q[3] -> meas[3];
    """

    task_info = {
        "circuit": qasm,
        "transpile": True,
        "qpu_name": 'Baihua',
        "optimization_level": 3,
        "task_type": 'qc',
        "vqpu_preferred": 'priority'
    }

    compiled_info = call_compiler_api(**task_info)

    compiled_openqasm = compiled_info[0]

    task = {'chip': 'Baihua',  # chip name
            'name': 'MyQuantumnTask',  # task name
            'circuit': compiled_openqasm,  # qasm
            'compile': False,  # No server-side compilation needed
            'options': {'compiler': 'qsteed'}
            }
    tid = tmgr.run(task, repeat=10)
    print('task id:', tid)


if __name__ == "__main__":
    test_send_quafu_task()
