from qsteed.qsteedcpp import SabreRouting as Cpp_SabreRouting
from qsteed.qsteedcpp import CouplingCircuit as Cpp_CouplingCircuit
from qsteed.qsteedcpp import Heuristic

from .dag_converter import *
from .qc_converter import *
from qsteed.qsteedcpp import dag_to_circuit, circuit_to_dag


class SabreRouting():
    """
    A class representing the SabreRouting algorithm.
    It can be used by transpiler directly as a pass.

    Args:
        heuristic (str): The heuristic to be used for routing. Can be one of "distance", "fidelity", or "mixture".

    Attributes:
        model: The model of backend.
        sabre_routing: The SabreRouting cpp_object.
        heuristic: The selected heuristic method.
    """

    def __init__(self, heuristic="distance"):
        self.model = None
        self.sabre_routing = None
        self.heuristic = heuristic

        if heuristic == "distance":
            self.heuristic = Heuristic.DISTANCE
        elif heuristic == "fidelity": 
            self.heuristic = Heuristic.FIDELITY
        elif heuristic == "mixture":
            self.heuristic = Heuristic.MIXTURE


    def set_model(self, model):
        """
        Set the model.

        Args:
            model: The model to be set.
        """
        self.model = model
        c_list = model.get_backend().get_property("coupling_list") 
        c_circuit = Cpp_CouplingCircuit(c_list)

        self.sabre_routing = Cpp_SabreRouting(c_circuit, self.heuristic)


    def run(self, dag):
        """
        Run the SabreRouting algorithm on the given DAG.

        Args:
            dag: The DAG to be routed.

        Returns:
            The routed DAG.
        """
        if isinstance(dag, DAGCircuit):
            raise NotImplementedError("DAGCircuit to Cpp_DAGCircuit conversion is not implemented yet.")
            dag = cppDag_to_dag(dag)
            dag = self.sabre_routing.run(dag)
            return cppDag_to_dag(dag)

        elif isinstance(dag, QuantumCircuit):
            # dag = QuantumCircuit_to_cppDag(dag)
            dag = circuit_to_dag(dag)
            dag = self.sabre_routing.run(dag)
            # return cppDag_to_QuantumCircuit(dag)
            return dag_to_circuit(dag, dag.num_qubits)