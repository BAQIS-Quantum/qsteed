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


from qsteed.resourcemanager.database_sql.initialize_database import initialize_database
from qsteed.resourcemanager.database_sql.update import update_sql
from qsteed.resourcemanager.database_sql.instantiating import update_memory_database


def backend_db_api(backend: str = None, chip_info_dict: dict = None):
    update_sql(backend, chip_info_dict)


def call_backend_db_api(backend=None,
                        chip_info_dict=None,
                        database: str = "update",):
    """
    Args:
        backend (str): Consistent with the chip's name in the configuration file config.ini.
        chip_info_dict (dict): e.g. see /tests/chipexample.json
        database (str): "update" or "initialize".
                        Selecting "initialize" will initialize all databases, so proceed with caution.
    """
    if database == "initialize":
        initialize_database()
    elif database == "update":
        backend_db_api(backend, chip_info_dict)
        print("Update in-memory database information.")
        update_memory_database()
