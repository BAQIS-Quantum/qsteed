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


from qsteed.resourcemanager.database_sql.sql_models import SubQPU, VQPU
from qsteed.resourcemanager.utils import virtual_qubits
from qsteed.graph.couplinggraph import CouplingGraph


def _same_name(name, expected_name):
    return name is not None and expected_name is not None and str(name).lower() == str(expected_name).lower()


def _edge_qubits(edges):
    if not edges:
        return set()
    return set(q[0] for q in edges) | set(q[1] for q in edges)


def query_qpu(qpus, qpu_name: str = None):
    # find_qpus = list(filter(lambda item: item.qpu_name == qpu_name, qpus))
    qpu_list = [item for item in qpus if _same_name(item.qpu_name, qpu_name)]
    if len(qpu_list) == 0:
        raise NameError("The " + qpu_name + " is not found.")
    else:
        return qpu_list


def query_stdqpu(stdqpus, stdqpu_name: str = None):
    stdqpu_list = [item for item in stdqpus if _same_name(item.stdqpu_name, stdqpu_name)]
    if len(stdqpu_list) == 0:
        raise NameError("The " + stdqpu_name + " is not found.")
    else:
        return stdqpu_list


def query_vqpu(vqpus, qpu_name: str = None, qubits_num: int = None):
    if qpu_name is None and qubits_num is not None:
        return [item for item in vqpus if item.qpu_name is not None and item.qubits_num == qubits_num]
    elif qpu_name is not None and qubits_num is not None:
        return [item for item in vqpus if _same_name(item.qpu_name, qpu_name) and item.qubits_num == qubits_num]
    else:
        raise ValueError("The qubits_num is not specified.")


def query_specified_vqpu(vqpus, qpu_name: str = None, qubits_list: list = None):
    if qpu_name is not None and qubits_list is not None:
        vqpu_list = [item for item in vqpus
                     if _same_name(item.qpu_name, qpu_name) and _edge_qubits(item.coupling_list) == set(qubits_list)]
        return vqpu_list
    else:
        raise ValueError("The qubits_list or qpu_name is not specified.")


def generate_specified_vqpu(qpus, qpu_name: str = None, qubits_list: list = None):
    if qpu_name is not None and qubits_list is not None:
        qpu = query_qpu(qpus, qpu_name=qpu_name)[0]
        substructure = [item for item in qpu.structure if item[0] in qubits_list and item[1] in qubits_list]
        coupling_list, vq_to_q = virtual_qubits(substructure)
        data = {'vqpu_name': None,
                'qpu_name': qpu.qpu_name,
                'backend_type': qpu.backend_type,
                'qubits_num': len(qubits_list),
                'coupling_list': coupling_list,
                'coupling_graph': CouplingGraph(coupling_list),
                'basis_gates': qpu.basis_gates,
                'status': qpu.status, 'pulse': qpu.pulse,
                'vq_to_q': vq_to_q
                }
        vqpu_list = [VQPU(**data)]
        return vqpu_list
    else:
        raise ValueError("The qubits_list or qpu_name is not specified.")


def query_subqpu(subqpus, qpu_name: str = None, qubits_num: int = None):
    if qpu_name is None and qubits_num is not None:
        return [item for item in subqpus if item.qpu_name is not None and item.qubits_num == qubits_num]
    elif qpu_name is not None and qubits_num is not None:
        return [item for item in subqpus if _same_name(item.qpu_name, qpu_name) and item.qubits_num == qubits_num]
    else:
        raise ValueError("The qubits_num is not specified.")


def query_specified_subqpu(subqpus, qpu_name: str = None, qubits_list: list = None):
    if qpu_name is not None and qubits_list is not None:
        subqpu_list = [item for item in subqpus
                       if _same_name(item.qpu_name, qpu_name)
                       and _edge_qubits(item.substructure_CAL) == set(qubits_list)]
        return subqpu_list
    else:
        raise ValueError("The qubits_list or qpu_name is not specified.")


def generate_specified_subqpu(qpus, qpu_name: str = None, qubits_list: list = None):
    if qpu_name is not None and qubits_list is not None:
        qpu = query_qpu(qpus, qpu_name=qpu_name)[0]
        substructure_CAL = [item for item in qpu.structure if item[0] in qubits_list and item[1] in qubits_list]
        data = {'subqpu_name': None,
                'qpu_name': qpu.qpu_name,
                'qpu_id': qpu.qpu_id,
                'backend_type': qpu.backend_type,
                'qubits_num': len(qubits_list),
                'basis_gates': qpu.basis_gates,
                'status': False,
                'pulse': qpu.pulse,
                'vq_to_subq': None,
                'subq_to_vq': None,
                'calibration_benchmark': 'calibration',
                'substructure_CAL': substructure_CAL,
                }
        subqpu_list = [SubQPU(**data)]
        return subqpu_list
    else:
        raise ValueError("The qubits_list or qpu_name is not specified.")
