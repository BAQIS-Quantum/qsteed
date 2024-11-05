from qsteed.passes.mapping.cpp_utils.sabre import SabreLayout as Cpp_SabreLayout
from qsteed.passes.mapping.cpp_utils.sabre import CouplingCircuit as Cpp_CouplingCircuit

from qsteed.passes.mapping.cpp_utils.sabre import Heuristic

from .dag_converter import *
from .qc_converter import *


class SabreLayout():
    """
    SabreLayout_ class represents a layout algorithm for quantum circuits using the SabreLayout technique.
    It can be used by transpiler directly as a pass.

    Args:
        heuristic (str): The heuristic to be used for layout optimization. Possible values are "distance", "fidelity", and "mixture".
        max_iterations (int): The maximum number of iterations for the layout optimization algorithm.

    Attributes:
        sabre_layout: The SabreLayout cpp_object used for layout optimization.
        model: The model of backend.
    """

    def __init__(self, heuristic="distance", max_iterations=3):
        self.sabre_layout = None
        self.model = None

        self.heuristic = self.choose_heuristic(heuristic)
        self.max_iterations = max_iterations


    def set_model(self, model):
        """
        Sets the model. Includes information such as backend and layout.

        Args:
            model: The model to be set.
        """
        self.model = model
        c_list = model.get_backend().get_property("coupling_list") 
        c_circuit = Cpp_CouplingCircuit(c_list)

        self.sabre_layout = Cpp_SabreLayout(c_circuit)
        self.sabre_layout.heuristic = self.heuristic
        self.sabre_layout.max_iterations = self.max_iterations


    def get_model(self):
        self.model._layout["final_layout"] = self.sabre_layout.get_model().final_layout.get_v2p()
        self.model._layout["initial_layout"] = self.sabre_layout.get_model().init_layout.get_v2p()
        # print(self.model)
        return self.model  


    def run(self, dag):
        """
        Runs the layout optimization algorithm on the given DAG circuit.

        Args:
            dag: The DAG circuit to be optimized.

        Returns:
            The optimized DAG circuit.
        """
        if isinstance(dag, DAGCircuit):
            dag = dag_to_cppDag(dag)
            dag = self.sabre_layout.run(dag)
            return cppDag_to_dag(dag)

        elif isinstance(dag, QuantumCircuit):
            dag = QuantumCircuit_to_cppDag(dag)
            dag = self.sabre_layout.run(dag)
            return cppDag_to_QuantumCircuit(dag)


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