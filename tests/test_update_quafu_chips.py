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
import os
from qsteed.apis.resourceDB_api import transform_quafu_chip, update_chip_api
from quark import Task

TOKEN = 'JeRkiDCGU57LD5CPHjj:JF`pj:fmpw5GIO8twWfNsX5/:K{N4dUNyRkM1FUPxZ{O{h{OypkJxiY[jxjJ1NkP5BkP4FEJyFUM3BUM3JENzJjPjRYZqKDMypkJtWnemynJtJjck6zZi6zdqGYZjClfpWIfjpkJzW3d2Kzf'


def test_update_quafu_chip():
    tmgr = Task(token=TOKEN)

    # 查看可用芯片及当前状态
    print(tmgr.status())

    # 获取指定芯片信息
    info = tmgr.backend("Baihua")

    chip_dict = transform_quafu_chip(info)
    chip_dict["system_id"] = 7   # 这里的system_id需要与/QSteed/config.ini里的一致

    update_chip_api(chip_name="Baihua", chip_info_dict=chip_dict)


if __name__ == "__main__":
    test_update_quafu_chip()
