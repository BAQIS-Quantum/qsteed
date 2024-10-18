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


def copy_config():
    home_dir = os.path.expanduser("~")
    config_dir = os.path.join(home_dir, "QSteed")
    config_file_user = os.path.join(config_dir, "config.ini")
    current_path = os.path.dirname(os.path.abspath(__file__))
    config_file_src = os.path.join(current_path, "config", "config.ini")
    print('config_file_src', config_file_src)

    if not os.path.exists(config_file_user):
        print(f"{config_file_user} not found, copying...")
        os.makedirs(config_dir, exist_ok=True)
        shutil.copy(config_file_src, config_file_user)
        print(f"config.ini has been copied to directory {config_dir}.")
    else:
        print(f"{config_file_user} already exists, skipping copy.")
