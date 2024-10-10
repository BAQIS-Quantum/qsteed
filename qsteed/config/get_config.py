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
import sys
import warnings


def get_config(user_config_file=None):
    current_os = sys.platform
    # Determine the location of the user configuration file.
    if user_config_file is None:
        if current_os.startswith('win'):
            user_config_file = os.path.expanduser(r'~\QSteed\config.ini')
        else:
            user_config_file = os.path.expanduser(r'~/QSteed/config.ini')

    # Check if the user configuration file exists, if not, use the default configuration file.
    if os.path.isfile(user_config_file):
        config_file = user_config_file
    else:
        current_directory = os.path.dirname(__file__)
        config_file = os.path.join(current_directory, "config.ini")
        warnings.warn("Use the default configuration file, "
                      "you can set your own configuration file in the directory: " + user_config_file)
    return config_file
