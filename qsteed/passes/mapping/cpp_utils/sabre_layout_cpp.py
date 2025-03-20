from qsteed.passes.mapping.cpp_utils.qsteedcpp import SabreLayout as Cpp_SabreLayout
from qsteed.passes.mapping.cpp_utils.qsteedcpp import CouplingCircuit as Cpp_CouplingCircuit
from typing import Union
from .dag_converter import *
from .qc_converter import *

from qsteed.passes.basepass import BasePass
from qsteed.graph.couplinggraph import CouplingGraph
from qsteed.passes.mapping.baselayout import Layout
from qsteed.passes.mapping.layout.overall_layout_dense import OverallLayoutDense
from qsteed.passes.mapping.layout.overall_layout_fidelity import OverallLayoutFidelity
from qsteed.passes.mapping.layout.overall_layout_random import OverallLayoutRandom
from qsteed.passes.datadict import DataDict
from qsteed.passes.model import Model


class SabreLayout(BasePass):
    """
    SabreLayout class represents a layout algorithm for quantum circuits using the SabreLayout technique.
    It can be used by transpiler directly as a pass.


    References:
        Gushu Li, Yufei Ding, and Yuan Xie. "Tackling the qubit mapping problem
        for NISQ-era quantum devices." ASPLOS ’19, 1001–1014 (ACM, 2019).
        URL https://doi.org/10.1145/3297858.3304023

    Args:
        heuristic (str): The heuristic to be used for layout optimization. Possible values are "distance", "fidelity", and "mixture".
        max_iterations (int): The maximum number of iterations for the layout optimization algorithm.

    Attributes:
        sabre_layout: The SabreLayout cpp_object used for layout optimization.
        model: The model of backend.
    """

    def __init__(self,
                 coupling_list: List = None,
                 heuristic: str = "distance",
                 routing_pass=None,
                 max_iterations=3,
                 sabre_initial_layout: Layout = None,
                 initial_layout_method: str = "random"):

        super().__init__()

        self.coupling_graph = None
        self.coupling_list = coupling_list
        self.routing_pass = routing_pass
        self.max_iterations = max_iterations
        self.sabre_initial_layout: Layout = sabre_initial_layout
        self.model: Model = None
        self.heuristic: str = heuristic
        self.initial_layout_method: str = initial_layout_method

        self._sabre_layout: Cpp_SabreLayout = None
        self._c_circuit: Cpp_CouplingCircuit = None

    def set_model(self, model):
        """
        Sets the model. Includes information such as backend and layout.

        Args:
            model: The model to be set.
        """
        self.model = model
        # self.coupling_list = model.get_backend().get_property("coupling_list")
        # self._c_circuit = Cpp_CouplingCircuit(self.coupling_list)
        self.coupling_graph = self.model.get_backend().get_property('coupling_graph')
        self._c_circuit = Cpp_CouplingCircuit(self.coupling_graph.coupling_list)

        if self.model.datadict is None:
            self.model.datadict = DataDict()

    def get_model(self):
        return self.model

    def run(self, circuit: Union[QuantumCircuit, DAGCircuit]) -> QuantumCircuit:
        """
        Runs the layout optimization algorithm on the given circuit.

        Args:
            circuit: The circuit to be optimized.

        Returns:
            The optimized circuit circuit.
        """

        if isinstance(circuit, DAGCircuit):
            circuit = dag_to_cppDag(circuit)
        elif isinstance(circuit, QuantumCircuit):
            circuit = QuantumCircuit_to_cppDag(circuit)
        else:
            raise TypeError('Error: SabreLayout pass only supports QuantumCircuit or DAGCircuit.')

        qubits_used = list(circuit.get_qubits_used())

        # Try to get Initial layout.
        if self.sabre_initial_layout is not None:
            self.model.set_layout({'initial_layout': self.sabre_initial_layout})
            qubits_list = list(self.sabre_initial_layout.p2v.keys())
            used_subgraph = self.coupling_graph.subgraph(qubits_list)
            self.model.set_used_subgraph(used_subgraph)

        elif self.model.get_layout()["final_layout"] is not None:
            # final_layout may come from the previous pass
            self.model.set_layout({'initial_layout': self.model.get_layout()["final_layout"]})
            qubits_list = list(self.model.get_layout()["final_layout"].p2v.keys())
            used_subgraph = self.coupling_graph.subgraph(qubits_list)
            self.model.set_used_subgraph(used_subgraph)

        if self.model.get_layout()["initial_layout"] is None:
            if len(qubits_used) == self.coupling_graph.num_qubits:
                layout = Layout()
                # Method1: Choose a trivial initial_layout.
                # layout.generate_trivial_layout(virtual_qubits=dag.circuit_qubits)
                # Method2: Choose a random initial_layout.
                layout.generate_random_layout(len(qubits_used), self.coupling_graph.num_qubits)
                self.model.set_layout({'initial_layout': layout})
                self.model.set_used_subgraph(self.coupling_graph)

            elif len(qubits_used) < self.coupling_graph.num_qubits:
                if self.initial_layout_method == 'random':
                    layout = OverallLayoutRandom(coupling_graph=self.coupling_graph, qubits_list=qubits_used)
                elif self.initial_layout_method == 'fidelity':
                    layout = OverallLayoutFidelity(coupling_graph=self.coupling_graph, qubits_list=qubits_used)
                elif self.initial_layout_method == 'dense':
                    layout = OverallLayoutDense(coupling_graph=self.coupling_graph, qubits_list=qubits_used)
                else:
                    raise ValueError("initial_layout_method can only be 'random', 'fidelity' or 'dense'.")

                subgraph = layout.overall_layout()
                weight = list(list(subgraph.edges(data=True))[0][2].keys())[0]
                sub_coupling_list = [(u, v, data[weight]) for u, v, data in subgraph.edges(data=True)]
                used_subgraph = CouplingGraph(sub_coupling_list)
                self.model.set_layout({'initial_layout': layout})
                self.model.set_used_subgraph(used_subgraph)
            else:
                raise ValueError("The required qubits are more than the number of physical qubits.")

        self._c_circuit = Cpp_CouplingCircuit(self.model.get_backend().get_property("used_subgraph").coupling_list)

        # Initialize SabreLayout C++ Class
        self._sabre_layout = Cpp_SabreLayout(
            self._c_circuit,
            self.max_iterations,
            self.heuristic,
            self.model.get_layout()["initial_layout"].v2p,
        )

        # Run
        optimized_circuit = self._sabre_layout.run(circuit)
        optimized_circuit = cppDag_to_QuantumCircuit(optimized_circuit)

        self.model._layout["initial_layout"] = Layout(self._sabre_layout.get_model().initial_layout.get_v2p())
        self.model._layout["final_layout"] = Layout(self._sabre_layout.get_model().final_layout.get_v2p())
        self.model.datadict['add_swap_count'] = self._sabre_layout.get_add_swap_count()
        return optimized_circuit
