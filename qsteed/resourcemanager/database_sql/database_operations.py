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
import os

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


def _get_config_dict():
    config_file = get_config()
    config = configparser.ConfigParser()
    config.read(config_file)
    return config_to_dict(config)


def get_database_config():
    """Read database settings and normalize for mysql/sqlite."""
    config_dict = _get_config_dict()
    database_section = config_dict.get('Database', {})
    db_type = str(database_section.get('db_type', 'mysql')).lower()

    if db_type == 'sqlite':
        sqlite_config = database_section.get('sqlite_config', {})
        sqlite_path = sqlite_config.get('path', '~/QSteed/qsteed.db')
        sqlite_path = os.path.abspath(os.path.expanduser(sqlite_path))
        return {'db_type': 'sqlite', 'sqlite_path': sqlite_path}

    mysql_config = config_dict['MySQL']['mysql_config']
    return {'db_type': 'mysql', 'mysql_config': mysql_config}


def get_database_uri():
    db_config = get_database_config()
    if db_config['db_type'] == 'sqlite':
        sqlite_dir = os.path.dirname(db_config['sqlite_path'])
        if sqlite_dir:
            os.makedirs(sqlite_dir, exist_ok=True)
        return f"sqlite:///{db_config['sqlite_path']}"

    mysql_config = db_config['mysql_config']
    return 'mysql+pymysql://' + mysql_config["user"] + ':' \
           + mysql_config["password"] + '@' + mysql_config["host"] + \
           '/' + mysql_config["database"]


def check_database(db_name: str = None):
    """Check if the database exists"""
    db_config = get_database_config()
    if db_config['db_type'] == 'sqlite':
        sqlite_path = db_config['sqlite_path']
        return (sqlite_path,) if os.path.exists(sqlite_path) else None

    mysql_config = db_config['mysql_config']
    connection = pymysql.connect(
        host=mysql_config['host'],
        user=mysql_config['user'],
        password=mysql_config['password']
    )

    cursor = connection.cursor()
    check_database_query = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = %s"
    cursor.execute(check_database_query, (db_name,))
    result = cursor.fetchone()
    cursor.close()
    connection.close()
    return result


def database_operations(mysql_config: dict = None, create=True, reset=False, delete=False):
    if mysql_config is not None:
        db_config = {'db_type': 'mysql', 'mysql_config': mysql_config}
    else:
        db_config = get_database_config()

    if db_config['db_type'] == 'sqlite':
        sqlite_path = db_config['sqlite_path']
        sqlite_dir = os.path.dirname(sqlite_path)
        if sqlite_dir:
            os.makedirs(sqlite_dir, exist_ok=True)

        if reset and os.path.exists(sqlite_path):
            os.remove(sqlite_path)
            print(f"SQLite database {sqlite_path} reset successfully.")
        elif delete and os.path.exists(sqlite_path):
            os.remove(sqlite_path)
            print(f"SQLite database {sqlite_path} deleted successfully.")
        elif create:
            if not os.path.exists(sqlite_path):
                open(sqlite_path, 'a', encoding='utf-8').close()
                print(f"SQLite database {sqlite_path} created successfully.")
            else:
                print(f"SQLite database {sqlite_path} already exists.")
        return

    mysql_config = db_config['mysql_config']
    connection = pymysql.connect(
        host=mysql_config['host'],
        user=mysql_config['user'],
        password=mysql_config['password']
    )

    cursor = connection.cursor()
    check_database_query = "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = %s"
    cursor.execute(check_database_query, (mysql_config['database'],))
    result = cursor.fetchone()

    if result is None and create is True:
        create_database_query = fr"CREATE DATABASE {mysql_config['database']}"
        cursor.execute(create_database_query)
        print(fr"Database {mysql_config['database']} does not exist, created successfully.")

    elif result is not None and create is True:
        print(fr"Database {mysql_config['database']} already exists.")

    elif delete is True and result:
        drop_database_query = fr"DROP DATABASE {mysql_config['database']}"
        cursor.execute(drop_database_query)
        print(fr"Database {mysql_config['database']} deleted successfully.")

    elif reset is True and result:
        drop_database_query = fr"DROP DATABASE {mysql_config['database']}"
        cursor.execute(drop_database_query)

        create_database_query = fr"CREATE DATABASE {mysql_config['database']}"
        cursor.execute(create_database_query)
        print(fr"Database {mysql_config['database']} reset successfully.")

    cursor.close()
    connection.close()


def delete_db(db_name: str = None):
    db_config = get_database_config()
    if db_config['db_type'] == 'sqlite':
        sqlite_path = db_name or db_config['sqlite_path']
        sqlite_path = os.path.abspath(os.path.expanduser(sqlite_path))
        if os.path.exists(sqlite_path):
            os.remove(sqlite_path)
        return

    mysql_config = db_config['mysql_config']
    connection = pymysql.connect(
        host=mysql_config['host'],
        user=mysql_config['user'],
        password=mysql_config['password']
    )

    cursor = connection.cursor()
    drop_database_query = fr"DROP DATABASE {db_name}"
    cursor.execute(drop_database_query)
    cursor.close()
    connection.close()
