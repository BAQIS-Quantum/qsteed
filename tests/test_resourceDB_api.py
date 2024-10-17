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
from qsteed.apis.resourceDB_api import update_chip_api
import json
import os


class TestDatabaseSetup:
    """Test cases for setting up the initial database."""

    def test_initialize_database(self):
        """Test the database initialization process."""
        # When deploying on a new computer, you need to run initialize_database() first to set up the initial database.
        initialize_database()

    def test_update_chip_api(self):
        """Test calling the backend DB API with example chip info."""
        chip_file = os.path.join(os.path.dirname(__file__), 'dongling.json')
        with open(chip_file, 'r') as file:
            data_dict = json.load(file)
        update_chip_api('dongling', data_dict)

        chip_file = os.path.join(os.path.dirname(__file__), 'chipexample.json')
        with open(chip_file, 'r') as file:
            data_dict = json.load(file)
        update_chip_api('example', data_dict)


if __name__ == "__main__":
    t = TestDatabaseSetup()
    t.test_initialize_database()
    t.test_update_chip_api()
