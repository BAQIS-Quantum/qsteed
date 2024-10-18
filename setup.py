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

import os
import shutil
from skbuild import setup
from setuptools.command.install import install
from setuptools import find_packages


class InstallCommand(install):
    def run(self):
        install.run(self)

        home_dir = os.path.expanduser("~")
        config_dir = os.path.join(home_dir, "QSteed")
        config_file_user = os.path.join(config_dir, "config.ini")
        config_file_src = os.path.join("qsteed", "config", "config.ini")

        if not os.path.exists(config_file_user):
            print(f"{config_file_user} not found, copying...")
            os.makedirs(config_dir, exist_ok=True)
            shutil.copy(config_file_src, config_file_user)
        else:
            print(f"{config_file_user} already exists, skipping copy.")


here = os.path.abspath(os.path.dirname(__file__))

variables = {}
with open(os.path.join(here, 'qsteed', "version.py"), "r", encoding="utf-8") as f:
    code = f.read()
    exec(code, variables)

__version__ = variables.get("__version__")

with open(os.path.join(here, "README.md"), encoding="utf-8") as f:
    long_description = f.read()

requirements = [
    "dill>=0.3.8",
    "ipython>=8.14.0",
    "matplotlib>=3.5.2",
    "numpy>=1.20.3,<2.0.0",
    "networkx>=2.6.3",
    "scipy>=1.8.1",
    "Pillow>=10.4.0",
    "rich>=13.7.1",
    "graphviz>=0.14.2",
    "tabulate>=0.9.0",
    "sqlalchemy>=2.0.28",
    "flask>=3.0.2",
    "pymysql>=1.1.0",
    "flask_sqlalchemy>=3.1.1"
]

setup(
    name="qsteed",
    version=__version__,
    author="Xuhongze",
    author_email="xhzby1995@163.com",
    url="https://github.com/BAQIS-Quantum/QSteed",
    description="Quantum Software of Compilation for supporting Real Quantum device",
    long_description=long_description,
    long_description_content_type="text/markdown",
    keywords=["qsteed", "quantum compiling", "quantum computing", "quantum operating system"],
    install_requires=requirements,
    packages=find_packages(exclude=["test*"]),
    include_package_data=True,
    extras_require={"tests": ["pytest"]},
    python_requires=">=3.10",
    license="Apache-2.0 License",
    cmdclass={"install": InstallCommand},
    entry_points={
        'console_scripts': [
            'qsteed-config = qsteed.qsteed_config:copy_config',
            'qsteed-build_db = qsteed.first_build_db:first_build_db',
        ],
    },
)
