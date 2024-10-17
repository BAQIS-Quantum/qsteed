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


from sqlalchemy import ForeignKey
from sqlalchemy.orm import relationship
from qsteed.resourcemanager.database_sql.initialize_app_db import db


class SubQPU(db.Model):
    """Substructure QPU Backend
    """
    __tablename__ = 'SubQPU'

    id = db.Column(db.BIGINT, primary_key=True, autoincrement=True)
    subqpu_name = db.Column(db.String(255), index=True, unique=True)
    qpu_name = db.Column(db.String(255), index=True)
    qpu_id = db.Column(db.Integer, index=True)
    backend_type = db.Column(db.String(255))
    qubits_num = db.Column(db.Integer, index=True)
    basis_gates = db.Column(db.PickleType)
    status = db.Column(db.Boolean)
    pulse = db.Column(db.Boolean)
    vq_to_subq = db.Column(db.PickleType)
    subq_to_vq = db.Column(db.PickleType)
    calibration_benchmark = db.Column(db.String(255))
    benchmark_time = db.Column(db.DateTime)
    substructure_CAL = db.Column(db.PickleType)
    substructure_BM = db.Column(db.PickleType)
    substructure_reset = db.Column(db.PickleType)

    VQPU_DBid = db.Column(db.BIGINT, ForeignKey('VQPU.id'))
    StdQPU_DBid = db.Column(db.BIGINT, ForeignKey('StdQPU.id'))

    sub2v = relationship("VQPU", back_populates="v2sub", foreign_keys=[VQPU_DBid])
    sub2std = relationship("StdQPU", back_populates="std2sub", foreign_keys=[StdQPU_DBid])


class StdQPU(db.Model):
    """Standard QPU Backend
    """
    __tablename__ = 'StdQPU'

    id = db.Column(db.BIGINT, primary_key=True, autoincrement=True)
    stdqpu_name = db.Column(db.String(255), index=True, unique=True)
    qpu_name = db.Column(db.String(255), index=True, unique=True)
    qpu_id = db.Column(db.Integer, index=True, unique=True)
    qubits_info = db.Column(db.PickleType)
    topological_structure = db.Column(db.PickleType)
    backend_type = db.Column(db.String(255))
    backend_shape = db.Column(db.PickleType)
    crosstalk_constraint = db.Column(db.PickleType)
    standard_graph = db.Column(db.PickleType)
    basis_gates = db.Column(db.PickleType)
    pulse = db.Column(db.Boolean)
    node_to_qubit = db.Column(db.PickleType)
    qubit_to_node = db.Column(db.PickleType)
    structure = db.Column(db.PickleType)
    # complete_qubits = db.Column(db.PickleType)
    # available_structure = db.Column(db.PickleType)
    # available_qubits = db.Column(db.PickleType)
    calibration_time = db.Column(db.DateTime)
    benchmark_time = db.Column(db.DateTime)
    benchmark_data = db.Column(db.PickleType)

    QPU_DBid = db.Column(db.BIGINT, ForeignKey('QPU.id'))

    std2qpu = relationship("QPU", back_populates='qpu2std', foreign_keys=[QPU_DBid])
    std2sub = relationship('SubQPU', back_populates='sub2std',
                              foreign_keys=[SubQPU.StdQPU_DBid])


class QPU(db.Model):
    """QPU Backend
    """
    __tablename__ = 'QPU'

    id = db.Column(db.BIGINT, primary_key=True, autoincrement=True)
    qpu_name = db.Column(db.String(255), index=True, unique=True)
    qpu_id = db.Column(db.Integer, index=True, unique=True)
    backend_type = db.Column(db.String(255))
    qubits_num = db.Column(db.Integer)
    qubits_info = db.Column(db.PickleType)
    topological_structure = db.Column(db.PickleType)
    basis_gates = db.Column(db.PickleType)
    used_qubits = db.Column(db.PickleType)
    unused_qubits = db.Column(db.PickleType)
    status = db.Column(db.String(255))
    pulse = db.Column(db.Boolean)
    qubit_to_int = db.Column(db.PickleType)
    int_to_qubit = db.Column(db.PickleType)
    structure = db.Column(db.PickleType)
    calibration_time = db.Column(db.DateTime)
    benchmark_time = db.Column(db.DateTime)
    benchmark_data = db.Column(db.PickleType)
    priority_qubits = db.Column(db.PickleType)
    tasks_num = db.Column(db.BIGINT)
    executing_tasks_list = db.Column(db.PickleType)
    assigned_tasks_list = db.Column(db.PickleType)
    estimated_free_time = db.Column(db.FLOAT)

    qpu2std = relationship('StdQPU', back_populates='std2qpu',
                              foreign_keys=[StdQPU.QPU_DBid])


class VQPU(db.Model):
    """Virtualization QPU Backend
    """
    __tablename__ = 'VQPU'

    id = db.Column(db.BIGINT, primary_key=True, autoincrement=True)
    vqpu_name = db.Column(db.String(255), index=True, unique=True)
    qpu_name = db.Column(db.String(255), index=True)
    backend_type = db.Column(db.String(255))
    qubits_num = db.Column(db.Integer)
    coupling_list = db.Column(db.PickleType)
    coupling_graph = db.Column(db.PickleType)
    basis_gates = db.Column(db.PickleType)
    status = db.Column(db.String(255))
    pulse = db.Column(db.Boolean)
    vq_to_q = db.Column(db.PickleType)

    v2sub = relationship('SubQPU', back_populates='sub2v',
                            foreign_keys=[SubQPU.VQPU_DBid])
