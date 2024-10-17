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


from flask import Flask
from qsteed.resourcemanager.database_sql.sql_models import QPU, SubQPU, VQPU, StdQPU
from qsteed.resourcemanager.database_sql.initialize_app_db import app

SubQPUs = []
StdQPUs = []
QPUs = []
VQPUs = []


def get_qpu():
    return QPUs


def get_stdqpu():
    return StdQPUs


def get_subqpu():
    return SubQPUs


def get_vqpu():
    return VQPUs


def instantiating_qpu(app: Flask):
    with app.app_context():
        global QPUs
        if len(QPUs) == 0:
            QPUs = QPU.query.all()
        return QPUs


def instantiating_stdqpu(app: Flask):
    with app.app_context():
        global StdQPUs
        if len(StdQPUs) == 0:
            StdQPUs = StdQPU.query.all()
        return StdQPUs


def instantiating_subqpu(app: Flask):
    with app.app_context():
        global SubQPUs
        if len(SubQPUs) == 0:
            SubQPUs = SubQPU.query.all()
        return SubQPUs


def instantiating_vqpu(app: Flask):
    with app.app_context():
        global VQPUs
        if len(VQPUs) == 0:
            VQPUs = VQPU.query.all()
        return VQPUs


def update_memory_database():
    with app.app_context():
        global QPUs, StdQPUs, SubQPUs, VQPUs
        QPUs = QPU.query.all()
        VQPUs = VQPU.query.all()
        StdQPUs = StdQPU.query.all()
        SubQPUs = SubQPU.query.all()
        return QPUs, StdQPUs, SubQPUs, VQPUs


instantiating_qpu(app)
instantiating_subqpu(app)
instantiating_stdqpu(app)
instantiating_vqpu(app)
