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

import networkx as nx

from qsteed.resourcemanager.build_library import BuildLibrary
from qsteed.resourcemanager.database_sql.sql_models import QPU, SubQPU, StdQPU, VQPU
from qsteed.resourcemanager.database_sql.initialize_app_db import db
from qsteed.config.get_config import get_config
from qsteed.graph.couplinggraph import CouplingGraph
from qsteed.resourcemanager.utils import virtual_qubits
import warnings


# import matplotlib
# matplotlib.use('Agg')

# CONFIG_FILE = get_config()
# CONFIG = configparser.ConfigParser()
# CONFIG.read(CONFIG_FILE)
#
# BACKENDS_SHAPE = eval(CONFIG['ChipsShape']['chips_shape'])


def get_backend_shape():
    CONFIG_FILE = get_config()
    CONFIG = configparser.ConfigParser()
    CONFIG.read(CONFIG_FILE)

    BACKENDS_SHAPE = eval(CONFIG['ChipsShape']['chips_shape'])
    return BACKENDS_SHAPE


def save_qpu_data(chip_info, qpu_id=None):
    """

    Args:
        chip_info:
        qpu_id:

    Returns:

    """
    # chip_info = backend_info.get_chip_info
    # chip_info = backend_info['full_info']

    build_lib = BuildLibrary(backend=chip_info.name)

    int_to_qubit, qubit_to_int, structure = build_lib.get_structure(
        chip_info.topological_structure, chip_info.all_qubits_info)
    data = {'qpu_name': chip_info.name,
            'qpu_id': chip_info.system_id,
            'backend_type': 'superconducting',
            'qubits_num': chip_info.qubit_num,
            'qubits_info': chip_info.qubits_info,
            'topological_structure': chip_info.topological_structure,
            'basis_gates': chip_info.basis_gates,
            'used_qubits': None,
            'unused_qubits': None,
            'status': chip_info.system_status,
            'pulse': False,
            'qubit_to_int': qubit_to_int,
            'int_to_qubit': int_to_qubit,
            'structure': structure,
            'calibration_time': chip_info.calibration_time,
            'benchmark_time': None,
            'benchmark_data': None,
            'priority_qubits': chip_info.priority_qubits,
            }

    if qpu_id is None:  # Create data
        obj = QPU(**data)
        db.session.add(obj)
        db.session.commit()
    else:  # Update data
        data.update({'id': qpu_id})
        db.session.bulk_update_mappings(QPU, [data])
        db.session.commit()


def save_stdqpu_data(qpu: QPU = None, stdqpu_id=None, calibration_reset=False, benchmark_reset=False):
    """

    Args:
        qpu:
        reset:

    Returns:

    """
    node_to_qubit = _node_to_qubit(qpu)
    qubit_to_node = {v: k for k, v in node_to_qubit.items()}
    data = {'stdqpu_name': None,
            'qpu_name': qpu.qpu_name,
            'qpu_id': qpu.qpu_id,
            'qubits_info': qpu.qubits_info,
            'topological_structure': qpu.topological_structure,
            'backend_type': qpu.backend_type,
            'basis_gates': qpu.basis_gates,
            'pulse': qpu.pulse,
            'structure': qpu.structure,
            # 'complete_qubits': list(qpu.int_to_qubit.keys()),
            'calibration_time': qpu.calibration_time,
            'benchmark_time': qpu.benchmark_time,
            'benchmark_data': qpu.benchmark_data,
            'backend_shape': get_backend_shape()[qpu.qpu_name],
            'node_to_qubit': node_to_qubit,
            'qubit_to_node': qubit_to_node,
            'crosstalk_constraint': None,
            'standard_graph': None,
            # 'available_structure': None,
            # 'available_qubits': None,
            }

    if stdqpu_id is None:  # Create data
        obj = StdQPU(**data)
        db.session.add(obj)
        db.session.commit()
        stdqpu = StdQPU.query.filter_by(qpu_name=qpu.qpu_name).first()
        standard_graph = build_standard_graph(stdqpu)
        standard_graph = qpu_embed_stdqpu(stdqpu, standard_graph)
        stdqpu.standard_graph = standard_graph
        db.session.commit()
    elif calibration_reset:  # Update all data
        data.update({'id': stdqpu_id})
        db.session.bulk_update_mappings(StdQPU, [data])
        db.session.commit()
        stdqpu = StdQPU.query.filter_by(qpu_name=qpu.qpu_name).first()
        standard_graph = build_standard_graph(stdqpu)
        standard_graph = qpu_embed_stdqpu(stdqpu, standard_graph)
        stdqpu.standard_graph = standard_graph
        db.session.commit()
    elif not calibration_reset and benchmark_reset:  # Update benchmark data
        data.update({'id': stdqpu_id})
        db.session.bulk_update_mappings(StdQPU, [data])
        db.session.commit()


def save_subqpu_data(qpu: QPU = None, reset=False):
    """

    Args:
        qpu:
        reset:

    Returns:

    """
    data = {'subqpu_name': None,
            'qpu_name': qpu.qpu_name,
            'qpu_id': qpu.qpu_id,
            'backend_type': qpu.backend_type,
            'qubits_num': None,
            # 'coupling_list': None,
            # 'coupling_graph': None,
            'basis_gates': qpu.basis_gates,
            'status': False,
            'pulse': qpu.pulse,
            'vq_to_subq': None,
            'subq_to_vq': None,
            'benchmark_time': None,
            'calibration_benchmark': None,
            'substructure_CAL': None,
            'substructure_BM': None,
            'substructure_reset': None,
            }

    if reset is True:
        # If reset is required, delete the original data, re-create SubQPU backend
        SubQPU.query.filter_by(qpu_name=qpu.qpu_name).delete()
        db.session.commit()
        build_lib = BuildLibrary(backend=qpu.qpu_name)
        substructure_CAL_dict = build_lib.build_substructure_library(qpu.structure, qpu.int_to_qubit,
                                                                     eval(qpu.priority_qubits))
        # substructure_CAL_dict = build_lib.build_substructure_library(qpu.structure, qpu.int_to_qubit,
        #                                                              PRIORITY_REGIONS[qpu.qpu_name])
        for qubits_num, substructure_CAL_list in substructure_CAL_dict.items():
            for substructure_CAL in substructure_CAL_list:
                data['qubits_num'] = qubits_num
                data['calibration_benchmark'] = 'calibration'
                data['substructure_CAL'] = substructure_CAL
                obj = SubQPU(**data)
                db.session.add(obj)
                db.session.commit()

        # save_benchmark_substructure(data, qpu)

    else:
        SubQPU.query.filter_by(qpu_name=qpu.qpu_name, calibration_benchmark='benchmark').delete()
        db.session.commit()
        # save_benchmark_data(qpu)
        # save_benchmark_substructure(data, qpu)

    # Get all records and sort them in ascending order by primary key value.
    records = SubQPU.query.order_by(SubQPU.id).all()
    # Update primary key values one by one to consecutive integers.
    new_id = 1
    for record in records:
        record.id = new_id
        db.session.add(record)
        new_id += 1
    db.session.commit()


def save_vqpu_data(subqpu: SubQPU = None, cal_bm: str = 'cal'):
    """

    Args:
        subqpu:
        cal_bm:
    Returns:

    """
    # VQPU.query.filter_by(qpu_name=subqpus[0].qpu_name).delete()
    # db.session.commit()
    data = {'vqpu_name': None,
            'qpu_name': subqpu.qpu_name,
            'backend_type': subqpu.backend_type,
            'qubits_num': subqpu.qubits_num,
            'coupling_list': None,
            'coupling_graph': None,
            'basis_gates': subqpu.basis_gates,
            'status': subqpu.status,
            'pulse': subqpu.pulse,
            'vq_to_q': None,
            }

    if cal_bm == 'cal' and subqpu.substructure_CAL is not None:
        data['coupling_list'], data['vq_to_q'] = virtual_qubits(subqpu.substructure_CAL)
        data['coupling_graph'] = CouplingGraph(data['coupling_list'])
    elif cal_bm == 'bm' and subqpu.substructure_BM is not None:
        data['coupling_list'], data['vq_to_q'] = virtual_qubits(subqpu.substructure_BM)
        data['coupling_graph'] = CouplingGraph(data['coupling_list'])
    elif cal_bm == 'bm' and subqpu.substructure_BM is None and subqpu.substructure_CAL is not None:
        data['coupling_list'], data['vq_to_q'] = virtual_qubits(subqpu.substructure_CAL)
        data['coupling_graph'] = CouplingGraph(data['coupling_list'])
        warnings.warn("substructure_BM is None, coupling_list uses substructure_CAL data.")
    else:
        raise ValueError("cal_bm can only be 'cal' or 'bm'.")

    obj = VQPU(**data)
    db.session.add(obj)
    db.session.commit()


# def save_benchmark_substructure(data, qpu: QPU = None):
#     ghz = GHZBenchmark(qpu)
#     subqpus = SubQPU.query.filter_by(qpu_name=qpu.qpu_name,
#                                             calibration_benchmark='calibration').all()
#     for subqpu in subqpus:
#         substructure_reset = ghz.get_substructure_reset(qpu.benchmark_data, subqpu.substructure_CAL)
#         subqpu.substructure_reset = substructure_reset
#         subqpu.benchmark_time = qpu.benchmark_time
#         db.session.commit()
#
#     # add new benchmark subQPU
#     substructure_BM_dict = ghz.get_substructure_BM(qpu.benchmark_data)
#     for qubits_num, substructure_BM_list in substructure_BM_dict.items():
#         for substructure_BM in substructure_BM_list:
#             data['qubits_num'] = qubits_num
#             data['calibration_benchmark'] = 'benchmark'
#             data['substructure_BM'] = substructure_BM
#             data['benchmark_time'] = qpu.benchmark_time
#             obj = SubQPU(**data)
#             db.session.add(obj)
#             db.session.commit()


# def save_benchmark_data(qpu: QPU = None):
#     ghz = GHZBenchmark(qpu)
#     taskid_dict = ghz.send_ghz_benchmark(shots=5000, benchmark_opt='all')
#     while True:
#         benchmark_data = ghz.process_ghz_benchmark(taskid_dict)
#         if benchmark_data is not None:
#             qpu.benchmark_time = taskid_dict['benchmark_time']
#             qpu.benchmark_data = benchmark_data
#             db.session.commit()
#             break
#         time.sleep(20)  # Check every 20 second


def build_standard_graph(stdqpu: StdQPU = None):
    row = get_backend_shape()[stdqpu.qpu_name]['row']
    column = get_backend_shape()[stdqpu.qpu_name]['column']
    standard_graph = nx.grid_2d_graph(row, column)

    mapping = {(i, j): (i + 1, j + 1) for i, j in standard_graph.nodes()}
    standard_graph = nx.relabel_nodes(standard_graph, mapping)

    node_labels = {node: 'available' for node in standard_graph.nodes()}
    nx.set_node_attributes(standard_graph, node_labels, 'label')

    edge_labels = {edge: 'connect' for edge in standard_graph.edges()}
    nx.set_edge_attributes(standard_graph, edge_labels, 'label')

    pos = {(i, j): (j, -i) for i, j in standard_graph.nodes()}
    nx.draw(standard_graph, pos, with_labels=True, font_weight='bold', node_color='lightblue', node_size=300)

    return standard_graph


def qpu_embed_stdqpu(stdqpu: StdQPU = None, standard_graph=None):
    row = get_backend_shape()[stdqpu.qpu_name]['row']
    column = get_backend_shape()[stdqpu.qpu_name]['column']
    dimension = get_backend_shape()[stdqpu.qpu_name]['dimension']

    update_node_labels = {}
    update_edge_labels = {}

    # nodes = standard_graph.nodes()
    # edges = standard_graph.edges()

    # qpu = stdqpu.std2qpu

    # qpu_nodes = []
    # node_to_qubit = {}
    # for qubit in stdqpu.qubits_info.keys():
    #     node = _map_string_to_tuple(qubit, dimension=dimension)
    #     qpu_nodes.append(node)
    #     node_to_qubit[node] = qubit
    #
    # qubit_to_node = {v: k for k, v in node_to_qubit.items()}

    qpu_nodes = list(stdqpu.node_to_qubit.keys())

    for node in standard_graph.nodes():
        if node not in qpu_nodes:
            update_node_labels.update({node: 'unavailable'})

    qpu_edges = []
    for edge in stdqpu.topological_structure.keys():
        qubits_list = edge.split('_')
        qpu_edges.append((_map_string_to_tuple(qubits_list[0], dimension=dimension),
                          _map_string_to_tuple(qubits_list[1], dimension=dimension)))

    for edge in standard_graph.edges():
        if edge not in qpu_edges:
            update_edge_labels.update({edge: 'disconnect'})

    nx.set_node_attributes(standard_graph, update_node_labels, 'label')
    nx.set_edge_attributes(standard_graph, update_edge_labels, 'label')

    return standard_graph


def _node_to_qubit(qpu: QPU = None):
    dimension = get_backend_shape()[qpu.qpu_name]['dimension']

    node_to_qubit = {}
    for qubit in qpu.qubits_info.keys():
        node = _map_string_to_tuple(qubit, dimension=dimension)
        node_to_qubit[node] = qubit

    return node_to_qubit


def _map_string_to_tuple(s, dimension=1):
    if dimension == 1:
        row = 1
        col = int(s[1:])
    else:
        numbers = s[1:]
        half_length = len(numbers) // 2
        row = int(numbers[:half_length])
        col = int(numbers[half_length:])
    tuple_node = (row, col)
    return tuple_node

