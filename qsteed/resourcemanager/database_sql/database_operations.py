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

import pymysql

from qsteed.config.config_to_dict import config_to_dict
from qsteed.config.get_config import get_config


def get_mysql_config():
    config_file = get_config()
    config = configparser.ConfigParser()
    config.read(config_file)
    config_dict = config_to_dict(config)
    mysql_config = config_dict['MySQL']['mysql_config']
    return mysql_config


def check_database(db_name: str = None):
    """Check if the database exists"""

    mysql_config = get_mysql_config()
    # Create database connection
    connection = pymysql.connect(
        host=mysql_config['host'],
        user=mysql_config['user'],
        password=mysql_config['password']
    )

    # Create cursor object
    cursor = connection.cursor()
    check_database_query = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = %s"
    cursor.execute(check_database_query, (db_name,))
    result = cursor.fetchone()
    cursor.close()
    connection.close()
    return result


def database_operations(mysql_config: dict = None, create=True, reset=False, delete=False):
    if mysql_config is None:
        mysql_config = get_mysql_config()
        # mysql_config = eval(mysql_config)

    # Create database connection
    connection = pymysql.connect(
        host=mysql_config['host'],
        user=mysql_config['user'],
        password=mysql_config['password']
    )

    # Create cursor object
    cursor = connection.cursor()

    # Check if the database exists
    check_database_query = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = %s"
    cursor.execute(check_database_query, (mysql_config['database'],))
    result = cursor.fetchone()

    # If the database does not exist, create the database
    if result is None and create is True:
        create_database_query = fr"CREATE DATABASE {mysql_config['database']}"
        cursor.execute(create_database_query)
        print(fr"Database {mysql_config['database']} does not exist, created successfully.")

    elif result is not None and create is True:
        print(fr"Database {mysql_config['database']} already exists.")

    # Delete the database
    elif delete is True and result:
        drop_database_query = fr"DROP DATABASE {mysql_config['database']}"
        cursor.execute(drop_database_query)
        print(fr"Database {mysql_config['database']} deleted successfully.")

    # If you reset the database, delete it and recreate it
    elif reset is True and result:
        drop_database_query = fr"DROP DATABASE {mysql_config['database']}"
        cursor.execute(drop_database_query)

        create_database_query = fr"CREATE DATABASE {mysql_config['database']}"
        cursor.execute(create_database_query)
        print(fr"Database {mysql_config['database']} reset successfully.")

    # Close cursor and database connection
    cursor.close()
    connection.close()


def delete_db(db_name: str = None):
    mysql_config = get_mysql_config()

    # Create database connection
    connection = pymysql.connect(
        host=mysql_config['host'],
        user=mysql_config['user'],
        password=mysql_config['password']
    )

    # Create cursor object
    cursor = connection.cursor()

    drop_database_query = fr"DROP DATABASE {db_name}"
    cursor.execute(drop_database_query)

    # Close cursor and database connection
    cursor.close()
    connection.close()
