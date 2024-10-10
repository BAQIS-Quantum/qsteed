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
from qsteed.resourcemanager.database_sql.backend_dbAPI import call_backend_db_api
import json


class TestDatabaseSetup:
    """Test cases for setting up the initial database."""

    def test_initialize_database(self):
        """Test the database initialization process."""
        # When deploying on a new computer, you need to run initialize_database() first to set up the initial database.
        initialize_database()

    def test_call_backend_db_api(self):
        """Test calling the backend DB API with example chip info."""
        with open('dongling.json', 'r') as file:
            data_dict = json.load(file)
        call_backend_db_api(backend='dongling', chip_info_dict=data_dict)

        with open('chipexample.json', 'r') as file:
            data_dict = json.load(file)
        call_backend_db_api(backend='example', chip_info_dict=data_dict)


if __name__ == "__main__":
    t = TestDatabaseSetup()
    t.test_initialize_database()
    t.test_call_backend_db_api()
