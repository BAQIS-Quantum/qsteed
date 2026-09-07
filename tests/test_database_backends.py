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

import importlib.util
import sys
import types
from contextlib import contextmanager
from pathlib import Path
from unittest.mock import patch


PACKAGE_ROOT = Path(__file__).resolve().parents[1] / "qsteed"
DATABASE_SQL_ROOT = PACKAGE_ROOT / "resourcemanager" / "database_sql"


def _load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[name] = module
    spec.loader.exec_module(module)
    return module


@contextmanager
def _isolated_database_modules(config_path):
    package_modules = {}
    for name in (
        "qsteed",
        "qsteed.config",
        "qsteed.resourcemanager",
        "qsteed.resourcemanager.database_sql",
    ):
        module = types.ModuleType(name)
        module.__path__ = []
        package_modules[name] = module

    get_config_module = types.ModuleType("qsteed.config.get_config")
    get_config_module.get_config = lambda: str(config_path)
    package_modules[get_config_module.__name__] = get_config_module

    with patch.dict(sys.modules, package_modules):
        _load_module(
            "qsteed.config.config_to_dict",
            PACKAGE_ROOT / "config" / "config_to_dict.py",
        )
        operations = _load_module(
            "qsteed.resourcemanager.database_sql.database_operations",
            DATABASE_SQL_ROOT / "database_operations.py",
        )
        yield operations


def _write_config(path, db_type, sqlite_path):
    path.write_text(
        "\n".join(
            (
                "[Database]",
                f'db_type = "{db_type}"',
                f'sqlite_config = {{"path": "{sqlite_path}"}}',
                "",
                "[MySQL]",
                'mysql_config = {"host": "db.example", "user": "tester", '
                '"password": "secret", "database": "qsteed_test"}',
            )
        ),
        encoding="utf-8",
    )


def test_sqlite_database_lifecycle(tmp_path):
    config_path = tmp_path / "config.ini"
    sqlite_path = tmp_path / "data" / "qsteed.db"
    _write_config(config_path, "sqlite", sqlite_path)

    with _isolated_database_modules(config_path) as operations:
        assert operations.get_database_config() == {
            "db_type": "sqlite",
            "sqlite_path": str(sqlite_path),
        }
        assert operations.get_database_uri() == f"sqlite:///{sqlite_path}"

        operations.database_operations(create=True)
        assert operations.check_database() == (str(sqlite_path),)

        operations.database_operations(reset=True)
        assert not sqlite_path.exists()

        operations.database_operations(create=True)
        operations.delete_db()
        assert not sqlite_path.exists()


def test_mysql_configuration_remains_supported(tmp_path):
    config_path = tmp_path / "config.ini"
    _write_config(config_path, "mysql", tmp_path / "unused.db")

    with _isolated_database_modules(config_path) as operations:
        database_config = operations.get_database_config()
        assert database_config["db_type"] == "mysql"
        assert database_config["mysql_config"]["database"] == "qsteed_test"
        assert operations.get_database_uri().startswith("mysql+pymysql://tester:")


def test_sqlite_models_use_autoincrement_compatible_primary_keys(tmp_path):
    config_path = tmp_path / "config.ini"
    sqlite_path = tmp_path / "models.db"
    _write_config(config_path, "sqlite", sqlite_path)

    with _isolated_database_modules(config_path):
        app_db = _load_module(
            "qsteed.resourcemanager.database_sql.initialize_app_db",
            DATABASE_SQL_ROOT / "initialize_app_db.py",
        )
        models = _load_module(
            "qsteed.resourcemanager.database_sql.sql_models",
            DATABASE_SQL_ROOT / "sql_models.py",
        )
        initialization = _load_module(
            "qsteed.resourcemanager.database_sql.initialize_database",
            DATABASE_SQL_ROOT / "initialize_database.py",
        )

        initialization.initialize_database()

        with app_db.app.app_context():
            assert models.QPU.query.one().id == 1
            assert models.StdQPU.query.one().id == 1
            assert models.SubQPU.query.one().id == 1
            assert models.VQPU.query.one().id == 1
