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

from setuptools import find_packages
from setuptools.command.install import install
from skbuild import setup


def get_requirements(filename: str):
    if not os.path.exists(filename):
        raise FileNotFoundError(f"{filename} not found.")

    with open(filename, 'r') as file:
        lines = (line.strip() for line in file)
        return [line for line in lines if line and not line.startswith("#")]


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

with open(os.path.join(here, 'qsteed', "VERSION.txt")) as version_file:
    __version__ = version_file.read().strip()

with open(os.path.join(here, "README.md"), encoding="utf-8") as f:
    long_description = f.read()

requirements = get_requirements("requirements.txt")

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
