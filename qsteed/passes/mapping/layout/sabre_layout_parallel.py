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

import copy
from typing import Union, List

from quafu import QuantumCircuit

from qsteed.dag.circuit_dag_convert import circuit_to_dag, dag_to_circuit, copy_dag
from qsteed.dag.dagcircuit import DAGCircuit
from qsteed.graph.couplinggraph import CouplingGraph
from qsteed.passes.basepass import BasePass
from qsteed.passes.datadict import DataDict
from qsteed.passes.mapping.baselayout import Layout
from qsteed.passes.mapping.routing.sabre_routing import SabreRouting
from qsteed.utils.reverse_circuit import reverse_circuit


class SabreLayoutParallel(BasePass):
    """In order to perform a "SABRE" on any given circuit, it needs to be iterated multiple times
     until convergence or a maximum number of iterations is reached, resulting in the final layout.
    """

    def __init__(self, coupling_list: List = None,
                 heuristic="distance",
                 routing_pass=None,
                 max_iterations=3,
                 sabre_initial_layout=None):
        """SabreLayout initializer.

        Args:
            coupling_list (List): qubit coupling structure, e.g. [(0,1,0.99), (1,0,0.99), (1,2,0.98),(2,1,0.95)]
            routing_pass (BasePass): the routing pass to use while iterating.
            max_iterations (int): number of forward-backward iterations.
                                  If max_iterations=0, only perform one forward SabreRouting
            sabre_initial_layout (Layout)
        """
        super().__init__()

        self.coupling_list = coupling_list
        self.coupling_graph = None

        self.routing_pass = routing_pass
        self.max_iterations = max_iterations
        self.sabre_initial_layout = sabre_initial_layout
        # self.model = ModelDict()
        self.model = None
        self.heuristic = heuristic

    def set_model(self, model):
        self.model = model
        # self.coupling_list = self.model.get_backend().get_property('coupling_list')
        self.coupling_graph = self.model.get_backend().get_property('coupling_graph')

        if self.model.datadict is None:
            self.model.datadict = DataDict()

    def get_model(self):
        return self.model

    def run(self, circ_dag: Union[QuantumCircuit, DAGCircuit]):
        """Run the SabreLayout pass on DAGCircuit.

        Args:
            circ_dag (DAGCircuit or QuantumCircuit): Find the layout of this circuit.

        Returns:
            new_circuit (QuantumCircuit): Quantum circuit satisfying hardware coupling structure.
        """

        if self.coupling_graph is None:
            if self.coupling_list is not None:
                coupling_graph = CouplingGraph(self.coupling_list)
                if coupling_graph.is_bidirectional is False:
                    coupling_graph.do_bidirectional()
                self.coupling_graph = coupling_graph
            else:
                raise ValueError("Error: There is no qubits coupling structure.")

        if isinstance(circ_dag, QuantumCircuit):
            dag = circuit_to_dag(circ_dag)
            # circuit = copy.deepcopy(circ_dag)
            circuit = circ_dag
        elif isinstance(circ_dag, DAGCircuit):
            # dag = copy.deepcopy(circ_dag)
            dag = copy_dag(circ_dag)
            circuit = dag_to_circuit(circ_dag, circ_dag.circuit_qubits)
        else:
            raise TypeError('Error: SabreLayout pass only supports QuantumCircuit or DAGCircuit.')

        if len(dag.qubits_used) > self.coupling_graph.num_qubits:
            raise ValueError("More virtual qubits exist than physical qubits.")

        measure_nodes = dag.get_measure_nodes()
        dag.remove_measure_nodes()

        layout = Layout()
        layout.generate_random_layout(len(dag.qubits_used), self.coupling_graph.num_qubits)
        self.model.set_layout({'initial_layout': layout})

        if self.routing_pass is None:
            # self.routing_pass = SabreRouting(self.model.get_backend().get_property('coupling_list'),
            #                                  heuristic=self.heuristic, modify_dag=False)
            self.routing_pass = SabreRouting(heuristic=self.heuristic, modify_dag=False)
        else:
            self.routing_pass.modify_dag = False

        rev_circuit = reverse_circuit(circuit)
        rev_dag = circuit_to_dag(rev_circuit)

        # Do forward-backward iterations.
        for _ in range(self.max_iterations):
            for _ in ("forward", "backward"):
                self.run_single(dag)
                # self.model.get_layout()["initial_layout"] = copy.deepcopy(self.model.get_layout()["final_layout"])
                # self.model.get_layout()["initial_layout"] = self.model.get_layout()["final_layout"]
                self.model.set_layout({'initial_layout': copy.deepcopy(self.model.get_layout()['final_layout'])})
                dag, rev_dag = rev_dag, dag

        # The last forward iteration obtains the final circuit.
        self.routing_pass.modify_dag = True
        physical_dag = self.run_single(dag)
        physical_circuit = dag_to_circuit(physical_dag, physical_dag.circuit_qubits)

        # Add measurement, i.e. mapping of physical qubits to classical qubits
        p2c = {}  # mapping for physical qubits to classic qubits
        for virtual, classic in measure_nodes[-1].pos.items():
            p2c[self.model.get_layout()['final_layout'].v2p[virtual]] = classic
        p2c = dict(sorted(p2c.items(), key=lambda item: item[1]))
        physical_circuit.measure(list(p2c.keys()), list(p2c.values()))

        return physical_circuit

    def run_single(self, dag):
        if hasattr(self.routing_pass, 'set_model'):
            self.routing_pass.set_model(self.model)

        new_dag = self.routing_pass.run(dag)

        if hasattr(self.routing_pass, 'get_model'):
            self.model = self.routing_pass.get_model()
        return new_dag
