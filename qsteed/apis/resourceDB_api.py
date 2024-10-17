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


from qsteed.resourcemanager.database_sql.backend_dbAPI import call_backend_db_api


def update_chip_api(chip_name: str = None, chip_info_dict=None):
    """Update chip data in the quantum computing resource virtualization database
       and create it if the chip does not exist.

    Args:
        chip_name (str): Consistent with the chip's name in the configuration file config.ini.
        chip_info_dict (dict): e.g. see /tests/chipexample.json
    """
    call_backend_db_api(chip_name, chip_info_dict)