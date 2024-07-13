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

from os import path

from setuptools import setup, find_packages

here = path.abspath(path.dirname(__file__))
with open(path.join(here, "README.md"), encoding="utf-8") as f:
    long_description = f.read()

requirements = [
    "dill>=0.3.8",
    "ipython>=8.14.0",
    "matplotlib>=3.5.2",
    "numpy>=1.20.3,<2.0.0",
    "networkx>=2.6.3",
    "scipy>=1.8.1",
    "Pillow>=10.4.0",
    "pyquafu>=0.4.1",
    "rich>=13.7.1",
    "graphviz>=0.14.2",
    "tabulate>=0.9.0",
]

setup(
    name="qsteed",
    version="0.1.1",
    author="Xuhongze",
    author_email="xhzby1995@163.com",
    url="https://github.com/BAQIS-Quantum/QSteed",
    description="Quantum Software of Compilation for supporting Real Quantum device",
    long_description=long_description,
    long_description_content_type="text/markdown",
    keywords=["qsteed", "quantum compiling", "quantum computing", "quantum operating system"],
    install_requires=requirements,
    packages=find_packages(),
    include_package_data=True,
    extras_require={"tests": ["pytest"]},
    python_requires=">=3.10",
    license="Apache-2.0 License",
)
