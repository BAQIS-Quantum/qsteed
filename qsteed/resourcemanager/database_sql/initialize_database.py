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


from qsteed.resourcemanager.database_sql.sql_models import QPU, SubQPU, StdQPU, VQPU
from qsteed.resourcemanager.database_sql.database_operations import database_operations
from qsteed.resourcemanager.database_sql.initialize_app_db import db, app


def initialize_database():
    with app.app_context():
        database_operations(reset=True)
        db.create_all()
        db.session.commit()
        initialize_qpu()
        initialize_stdqpu()
        initialize_subqpu()
        initialize_vqpu()


def initialize_qpu():
    obj = QPU()
    db.session.add(obj)
    db.session.flush()


def initialize_stdqpu():
    obj = StdQPU()
    db.session.add(obj)
    db.session.flush()


def initialize_subqpu():
    obj = SubQPU()
    db.session.add(obj)
    db.session.flush()


def initialize_vqpu():
    obj = VQPU()
    db.session.add(obj)
    db.session.flush()
