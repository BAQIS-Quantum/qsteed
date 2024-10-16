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


def get_version():
    here = os.path.abspath(os.path.dirname(__file__))
    parent_dir = os.path.dirname(here)
    version_file = os.path.join(parent_dir, "VERSION")
    if os.path.exists(version_file):
        with open(version_file, encoding="utf-8") as f:
            version = f.read().strip()
        return version
    else:
        print("VERSION not found!")
