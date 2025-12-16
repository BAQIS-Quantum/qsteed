from qsteed.qsteedcpp import DAGCircuit as Cpp_DAGCircuit
from qsteed.qsteedcpp import InstructionNode as Cpp_InstructionNode
from qsteed.qsteedcpp import MeasureNode as Cpp_MeasureNode
from qsteed.qsteedcpp import EdgeProperties
from qsteed.qsteedcpp.expression import Expr

from qsteed.dag.dagcircuit import DAGCircuit
from qsteed.dag.instruction_node import InstructionNode 


def dag_to_cppDag(dag: DAGCircuit) -> Cpp_DAGCircuit:
    """
    Converts a DAGCircuit object to a Cpp_DAGCircuit object.

    Args:
        dag (DAGCircuit): The input DAGCircuit object to be converted.

    Returns:
        Cpp_DAGCircuit: The converted Cpp_DAGCircuit object.
    """
    cppDag = Cpp_DAGCircuit()

    # Add node
    for index, node in enumerate(dag.nodes(data=True)):
        cpp_node = node_to_cppNode(node[0])
        cppDag.add_node(cpp_node)

    node_to_index = {node_name: index for index, node_name in enumerate(dag.nodes())}
    # Add edge
    for u, v, data in dag.edges(data=True):
        ep = EdgeProperties(int(data['label'][1]))
        cppDag.add_edge(node_to_index[u], node_to_index[v], ep)

    return cppDag


def node_to_cppNode(node: InstructionNode) -> Cpp_InstructionNode:
    """
    Converts an InstructionNode object to a Cpp_InstructionNode object.

    Args:
        node (InstructionNode): The InstructionNode object to be converted.

    Returns:
        Cpp_InstructionNode: The converted Cpp_InstructionNode object.
    """
    if node == -1:
        cpp_node = Cpp_InstructionNode()
        cpp_node.name = "start"
        return cpp_node
    elif node == float('inf'):
        cpp_node = Cpp_InstructionNode()
        cpp_node.name = "end"
        return cpp_node

    elif isinstance(node.pos, list):
        cpp_node = Cpp_InstructionNode()
        cpp_node.name = node.name
        cpp_node.qubit_pos = node.pos

        if node.paras:
            cpp_node.paras = node.paras
        if node.duration:
            cpp_node.duration = node.duration
        if node.unit:
            cpp_node.unit = node.unit
        return cpp_node

    elif isinstance(node.pos, dict):
        if node.pos:
            cpp_node = Cpp_MeasureNode(node.pos.keys(), node.pos.values())
        else:
            cpp_node = Cpp_MeasureNode()
        return cpp_node




def cppDag_to_dag(cppDag: Cpp_DAGCircuit) -> DAGCircuit:
    # Starting Label Index
    i = 0

    # A dictionary to store the last use of any qubit
    qubit_last_use = {}

    # Add the start node 
    dag = DAGCircuit()
    dag.add_nodes_from([(-1, {"color": "green"})])

    for vertex in cppDag.vertices():
        if cppDag.graph[vertex].name == "start" or cppDag.graph[vertex].name == "end":
            continue
        else:
            hashable_gate = cppNode_to_node(cppDag.graph[vertex], i)
            i += 1

            dag.add_node(hashable_gate, color="blue") 

            for qubit in hashable_gate.pos:
                if qubit in qubit_last_use:
                    dag.add_edge(qubit_last_use[qubit], hashable_gate, label=f'q{qubit}')
                else:
                    dag.add_edge(-1, hashable_gate, label=f'q{qubit}', color="green")

                qubit_last_use[qubit] = hashable_gate


    dag.add_nodes_from([(float('inf'), {"color": "red"})])
    for qubit in qubit_last_use:
        dag.add_edge(qubit_last_use[qubit], float('inf'), label=f'q{qubit}', color="red")

    dag.update_qubits_used()
    dag.update_cbits_used()
    dag.update_num_instruction_nodes()

    return dag


def cppNode_to_node(cpp_node: Cpp_InstructionNode, specific_label) -> InstructionNode:
    node = InstructionNode(cpp_node.name, cpp_node.qubit_pos, cpp_node.paras, cpp_node.duration, cpp_node.unit, label=specific_label)
    return node
