from typing import Union
from qsteed.passes.mapping.cpp_utils.sabre import SabreLayout as Cpp_SabreLayout
from qsteed.passes.mapping.cpp_utils.sabre import CouplingCircuit as Cpp_CouplingCircuit
from .dag_converter import *
from .qc_converter import *

from qsteed.passes.basepass import BasePass
from qsteed.graph.couplinggraph import CouplingGraph
from qsteed.passes.mapping.baselayout import Layout
from qsteed.passes.mapping.layout.dense_layout import DenseLayout
from qsteed.passes.mapping.layout.fidelity_layout import FidelityLayout
from qsteed.passes.mapping.layout.random_layout import RandomLayout
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
        self.sabre_initial_layout = sabre_initial_layout
        self.model :Model = None
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
    
        self.coupling_list = model.get_backend().get_property("coupling_list") 
        self._c_circuit = Cpp_CouplingCircuit(self.coupling_list)

        if self.coupling_graph is None:
            if self.coupling_list is not None:
                coupling_graph = CouplingGraph(self.coupling_list)
                if coupling_graph.is_bidirectional is False:
                    coupling_graph.do_bidirectional()
                self.coupling_graph = coupling_graph
            else:
                raise ValueError("Error: There is no qubits coupling structure.")
        
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

        elif self.model.get_layout()["initial_layout"] is None: 
            self.model.set_layout({'initial_layout': Layout()})


        # Initialize SabreLayout C++ Class
        self._sabre_layout = Cpp_SabreLayout(
            self._c_circuit,
            self.max_iterations,
            self.heuristic,
            self.model.get_layout()["initial_layout"].v2p,
        )


        # Run
        if isinstance(circuit, DAGCircuit):
            circuit = dag_to_cppDag(circuit)
            optimized_circuit = self._sabre_layout.run(circuit)
            optimized_circuit = cppDag_to_QuantumCircuit(optimized_circuit)
        elif isinstance(circuit, QuantumCircuit):
            circuit = QuantumCircuit_to_cppDag(circuit)
            optimized_circuit = self._sabre_layout.run(circuit)
            optimized_circuit =  cppDag_to_QuantumCircuit(optimized_circuit)
        else:
            raise TypeError('Error: SabreLayout pass only supports QuantumCircuit or DAGCircuit.')


        self.model._layout["initial_layout"] = Layout(self._sabre_layout.get_model().initial_layout.get_v2p())
        self.model._layout["final_layout"] = Layout(self._sabre_layout.get_model().final_layout.get_v2p())

        return optimized_circuit