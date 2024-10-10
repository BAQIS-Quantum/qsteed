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


from qsteed.backends.chipinfo import ChipInfo
from qsteed.resourcemanager.database_sql.sql_models import QPU, SubQPU, StdQPU, VQPU
from qsteed.resourcemanager.database_sql.build_sql import save_qpu_data, save_subqpu_data, save_stdqpu_data, \
    save_vqpu_data
from qsteed.resourcemanager.database_sql.initialize_app_db import app, db


def update_sql(backend: str = None, chip_info_dict: dict = None):
    with app.app_context():
        chip_info = ChipInfo(backend, chip_info_dict)
        chip_info.initialize_chip()
        db.create_all()

        # QPU
        qpu = QPU.query.filter_by(qpu_name=backend).first()
        if qpu:
            if str(qpu.calibration_time) == chip_info.calibration_time:
                print('Calibration information unchanged. Please check calibration time.')
            else:  # Update data
                print('Calibration information has changed, update QPU data')
                save_qpu_data(chip_info, qpu_id=qpu.id)
                qpu = QPU.query.filter_by(qpu_name=backend).first()
                # save_benchmark_data(qpu)

        else:  # Create data
            print('Create QPU data.')
            save_qpu_data(chip_info)
            qpu = QPU.query.filter_by(qpu_name=backend).first()
            # save_benchmark_data(qpu)

        # StdQPU
        stdqpu = StdQPU.query.filter_by(qpu_name=backend).first()
        if stdqpu:
            qpu = QPU.query.filter_by(qpu_name=backend).first()
            if str(qpu.calibration_time) == str(stdqpu.calibration_time):
                print('Calibration information unchanged, QPU/StdQPU data unchanged.')
            else:  # Update data
                print('Calibration information has changed, update stdQPU data.')
                save_stdqpu_data(qpu, stdqpu_id=stdqpu.id, calibration_reset=True)

            if str(qpu.benchmark_time) == str(stdqpu.benchmark_time):
                pass
                # print('Benchmark information unchanged.')
            else:  # Update data
                print('Calibration information has changed, update stdQPU data')
                save_stdqpu_data(qpu, stdqpu_id=stdqpu.id, calibration_reset=False, benchmark_reset=True)

        else:  # Create data
            print('Create StdQPU data.')
            save_stdqpu_data(qpu)

        qpu = QPU.query.filter_by(qpu_name=backend).first()
        stdqpu = StdQPU.query.filter_by(qpu_name=backend).first()
        qpu.qpu2std.append(stdqpu)
        stdqpu.std2qpu = qpu

        # SubQPU
        subqpu = SubQPU.query.filter_by(qpu_name=backend).first()
        if subqpu:
            print("Reset all SubQPU of the %s" % qpu.qpu_name + ", using calibration data from " + str(
                qpu.calibration_time))
            save_subqpu_data(qpu, reset=True)
            print("SubQPU data are updated successfully.")
        else:
            print("There is no SubQPU data for the %s, create SubQPU data." % qpu.qpu_name)
            save_subqpu_data(qpu, reset=True)
            print("SubQPU data are created successfully.")

        subqpus = SubQPU.query.filter_by(qpu_name=backend).all()
        stdqpu = StdQPU.query.filter_by(qpu_name=backend).first()
        for subqpu in subqpus:
            stdqpu.std2sub = [subqpu]

        # VQPU
        vqpu = VQPU.query.filter_by(qpu_name=backend).first()
        VQPU.query.filter_by(qpu_name=backend).delete()
        db.session.commit()
        if vqpu:
            print("Reset all VQPU of the %s" % qpu.qpu_name + ", using calibration data from " + str(
                qpu.calibration_time))
            for subqpu in subqpus:
                save_vqpu_data(subqpu, cal_bm='cal')
            print("VQPU data are updated successfully.")
        else:
            print("There is no VQPU data for the %s, create VQPU data." % qpu.qpu_name)
            for subqpu in subqpus:
                save_vqpu_data(subqpu, cal_bm='cal')
            print("VQPU data are created successfully.")

        # Get all records and sort them in ascending order by primary key value.
        records = VQPU.query.order_by(VQPU.id).all()
        # Update primary key values one by one to consecutive integers.
        new_id = 1
        for record in records:
            record.id = new_id
            db.session.add(record)
            new_id += 1
        db.session.commit()
