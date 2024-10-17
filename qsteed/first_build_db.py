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


def build_db():
    from qsteed.resourcemanager.database_sql.database_operations import database_operations
    database_operations(create=True)


def create_tables():
    from qsteed.resourcemanager.database_sql.initialize_app_db import db, app
    from qsteed.resourcemanager.database_sql.initialize_database import initialize_qpu, initialize_stdqpu, \
        initialize_subqpu, initialize_vqpu
    with app.app_context():
        print("Creating all empty tables...")
        db.create_all()
        db.session.commit()
        initialize_qpu()
        initialize_stdqpu()
        initialize_subqpu()
        initialize_vqpu()


def first_build_db():
    build_db()
    create_tables()
