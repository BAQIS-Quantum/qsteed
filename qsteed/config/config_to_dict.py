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


import json


def config_to_dict(config):
    """
    Convert a ConfigParser object to a nested dictionary.
    """
    config_dict = {}
    for section in config.sections():
        section_dict = {}
        for key, value in config.items(section):
            try:
                # Attempt to parse a string value as JSON to support complex types like dictionaries, lists, etc.
                section_dict[key] = json.loads(value)
            except json.JSONDecodeError:
                # If parsing fails, retain the original string.
                section_dict[key] = value
        config_dict[section] = section_dict
    return config_dict
