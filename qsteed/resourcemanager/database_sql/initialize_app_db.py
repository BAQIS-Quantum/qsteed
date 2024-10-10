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

from flask import Flask
from flask_sqlalchemy import SQLAlchemy

from qsteed.config.config_to_dict import config_to_dict
from qsteed.config.get_config import get_config

# def initialize_app_db(mysql_config: dict = None):
#     if mysql_config is None:
#         config_file = get_config()
#         config = configparser.ConfigParser()
#         config.read(config_file)
#         config_dict = config_to_dict(config)
#         mysql_config = config_dict['MySQL']['mysql_config']
#         # mysql_config = eval(mysql_config)
#
#     app = Flask(__name__)
#     app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql+pymysql://' + mysql_config["user"] + ':' \
#                                             + mysql_config["password"] + '@' + mysql_config["host"] + \
#                                             '/' + mysql_config["database"]
#     db = SQLAlchemy(app)
#     return app, db
#
#
# app, db = initialize_app_db()

db = SQLAlchemy()


def initialize_app_db(mysql_config: dict = None):
    if mysql_config is None:
        config_file = get_config()
        config = configparser.ConfigParser()
        config.read(config_file)
        config_dict = config_to_dict(config)
        mysql_config = config_dict['MySQL']['mysql_config']

    app = Flask(__name__)
    app.config['SQLALCHEMY_DATABASE_URI'] = 'mysql+pymysql://' + mysql_config["user"] + ':' \
                                            + mysql_config["password"] + '@' + mysql_config["host"] + \
                                            '/' + mysql_config["database"]
    db.init_app(app)
    return app


def create_app():
    """Application factory function."""
    app = initialize_app_db()
    with app.app_context():
        db.create_all()
    return app


app = create_app()
