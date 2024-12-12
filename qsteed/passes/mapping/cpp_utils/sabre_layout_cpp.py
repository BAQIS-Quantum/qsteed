from qsteed.passes.mapping.cpp_utils.sabre import SabreLayout as Cpp_SabreLayout
from qsteed.passes.mapping.cpp_utils.sabre import CouplingCircuit as Cpp_CouplingCircuit

from qsteed.passes.mapping.cpp_utils.sabre import Heuristic

from .dag_converter import *
from .qc_converter import *

from qsteed.passes.basepass import BasePass
from qsteed.passes.mapping.baselayout import Layout
from qsteed.passes.datadict import DataDict


class SabreLayout(BasePass):
    """
    SabreLayout_ class represents a layout algorithm for quantum circuits using the SabreLayout technique.
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

    def __init__(self, coupling_list: List = None,
                 heuristic: str = "distance",
                 routing_pass=None,
                 max_iterations=3,
                 sabre_initial_layout: Layout = None,
                 initial_layout_method: str = "random"):

        super().__init__()

        self.coupling_list = coupling_list
        self.routing_pass = routing_pass
        self.max_iterations = max_iterations
        self.sabre_initial_layout = sabre_initial_layout
        self.model = None
        self.heuristic = self.choose_heuristic(heuristic)
        self.initial_layout_method = initial_layout_method

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
        
        if self.model.datadict is None:
            self.model.datadict = DataDict()


    def get_model(self):
        return self.model


    def run(self, dag):
        """
        Runs the layout optimization algorithm on the given DAG circuit.

        Args:
            dag: The DAG circuit to be optimized.

        Returns:
            The optimized DAG circuit.
        """
        self.sabre_layout = Cpp_SabreLayout(self._c_circuit, self.heuristic, self.max_iterations)

        if isinstance(dag, DAGCircuit):
            dag = dag_to_cppDag(dag)
            dag = self.sabre_layout.run(dag)
            return cppDag_to_dag(dag)

        elif isinstance(dag, QuantumCircuit):
            dag = QuantumCircuit_to_cppDag(dag)
            dag = self.sabre_layout.run(dag)
            return cppDag_to_QuantumCircuit(dag)

        self.model._layout["final_layout"] = self.sabre_layout.get_model().final_layout.get_v2p()
        self.model._layout["initial_layout"] = self.sabre_layout.get_model().init_layout.get_v2p()


    def choose_heuristic(self, heuristic):
        """
        Chooses the heuristic for the layout optimization.

        Args:
            heuristic: The heuristic to be chosen.
        """
        if heuristic == "distance":
            return Heuristic.DISTANCE
        elif heuristic == "fidelity": 
            return Heuristic.FIDELITY
        elif heuristic == "mixture":
            return Heuristic.MIXTURE
        else:
            raise NameError("Heuristic %s not recongnized" %heuristic)