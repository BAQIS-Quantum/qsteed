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

import operator
import time
from functools import reduce

from qsteed.resourcemanager.database_sql.database_query import query_subqpu, query_vqpu
from qsteed.resourcemanager.database_sql.instantiating import get_qpu, get_subqpu, get_vqpu

qpus = get_qpu()
subqpus = get_subqpu()
vqpus = get_vqpu()


class TestDatabaseQuery:
    """Test cases for querying QPU, SubQPU, and VQPU."""

    def test_query_subqpu(self, qpu_name: str = "example", qubits_num: int = 3):
        """Test querying SubQPUs."""
        subqpu = query_subqpu(subqpus, qpu_name=qpu_name, qubits_num=qubits_num)

        assert subqpu is not None
        assert all(hasattr(s, 'substructure_CAL') for s in subqpu)

        print("All substructures with %s qubits." % qubits_num)
        for s in subqpu:
            print(s.substructure_CAL)

    def test_query_vqpu(self, qpu_name: str = "example", qubits_num: int = 3):
        """Test querying VQPUs."""
        vqpu = query_vqpu(vqpus, qpu_name=qpu_name, qubits_num=qubits_num)

        assert vqpu is not None
        assert all(hasattr(s, 'coupling_list') for s in vqpu)

        print("All vqpus with %s qubits." % qubits_num)
        for s in vqpu:
            print(s.coupling_list)

    def _sort_vqpus(self, _vqpus, sort_attribute=None):
        """Helper function to sort VQPUs based on a specified attribute."""

        def calculate_product(vqpu):
            return reduce(operator.mul, (item[2] for item in getattr(vqpu, sort_attribute)), 1)

        return sorted(_vqpus, key=calculate_product, reverse=True)

    def test_sort_vqpus(self, qpu_name: str = "example", qubits_num: int = 3):
        """Test sorting VQPUs by coupling list."""
        vqpu = query_vqpu(vqpus, qpu_name=qpu_name, qubits_num=qubits_num)

        assert vqpu is not None

        st = time.time()
        sorted_vqpus = self._sort_vqpus(vqpu, sort_attribute='coupling_list')
        elapsed_time = time.time() - st

        assert sorted_vqpus is not None
        assert elapsed_time < 1  # Example assertion for performance

        print("Sorted vqpus with %s qubits." % qubits_num)
        for s in sorted_vqpus:
            print(s.coupling_list)


if __name__ == "__main__":
    name = "example"
    num = 3
    t = TestDatabaseQuery()
    t.test_query_subqpu(qpu_name=name, qubits_num=num)
    t.test_query_vqpu(qpu_name=name, qubits_num=num)
    t.test_sort_vqpus(qpu_name=name, qubits_num=num)
