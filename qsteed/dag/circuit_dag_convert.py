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
from typing import Any

import networkx as nx
import numpy as np
from quafu import QuantumCircuit
from quafu.elements import Barrier, Delay, XYResonance
from quafu.elements.element_gates import *
from quafu.elements.element_gates.clifford import *
from quafu.elements.element_gates.pauli import *

from .dagcircuit import DAGCircuit
from .instruction_node import InstructionNode

GATE_CLASSES = {
    "x": XGate,
    "y": YGate,
    "z": ZGate,
    "h": HGate,
    "s": SGate,
    "sdg": SdgGate,
    "t": TGate,
    "tdg": TdgGate,
    "rx": RXGate,
    "ry": RYGate,
    "rz": RZGate,
    "id": IdGate,
    "sx": SXGate,
    "sxdg": SXdgGate,
    "sy": SYGate,
    "sydg": SYdgGate,
    "w": WGate,
    "sw": SWGate,
    "swdg": SWdgGate,
    "p": PhaseGate,
    "delay": Delay,
    "barrier": Barrier,
    "cx": CXGate,
    "cp": CPGate,
    "swap": SwapGate,
    "iswap": ISwapGate,
    "rxx": RXXGate,
    "ryy": RYYGate,
    "rzz": RZZGate,
    "cy": CYGate,
    "cz": CZGate,
    "cs": CSGate,
    "ct": CTGate,
    "cry": CRYGate,
    "crx": CRXGate,
    "crz": CRZGate,
    "xy": XYResonance,
    "ccx": ToffoliGate,
    "cswap": FredkinGate,
    "mcx": MCXGate,
    "mcy": MCYGate,
    "mcz": MCZGate,
    "u3": U3Gate,
}


def gate_to_node(input_gate, specific_label):
    """
    Transform a gate in a quantum circuit of quafu (excluding measure gates)
    into a node in the graph with a specific label.

    Args:
        input_gate: a gate in quantumcircuit of quafu(not include measure_gate)
        specific_label: the label of the node in the graph

    Returns:
        node: a node in the graph, with specific label. A node is a InstructionNode object.

    """
    gate = copy.deepcopy(input_gate)  # avoid modifying the original gate
    if not isinstance(gate.pos, list):  # if gate.pos is not a list, make it a list
        gate.pos = [gate.pos]

    # use getattr check 'paras' and other attributes if exist. if the attr doesn't exist,return None
    gate.paras = getattr(gate, 'paras', None) or None
    gate.duration = getattr(gate, 'duration', None) or None
    gate.unit = getattr(gate, 'unit', None) or None

    if gate.paras and not isinstance(gate.paras, list):  # if paras is True and not a list, make it a list
        gate.paras = [gate.paras]

    hashable_gate = InstructionNode(gate.name, gate.pos, gate.paras, gate.duration, gate.unit, label=specific_label)
    return hashable_gate


def circuit_to_dag(circuit: QuantumCircuit, measure_flag=True):
    """
    Building a DAG Graph using DAGCircuit from a QuantumCircuit

    Args:
        circuit: a QuantumCircuit object
        measure_flag: whether to add measure_gate node to the dag graph

    Returns:
        g: a DAGCircuit object

    example:
        from circuit_dag_convert import circuit_to_dag, dag_to_circuit, draw_dag
        from quafu import QuantumCircuit

        # Create a quantum circuit as an example that you can modify as needed
        circuit = QuantumCircuit(2)
        circuit.h(0)
        circuit.cnot(0, 1)

        # Build the dag graph
        dag = circuit_to_dag(circuit)  #  dag graph
    """

    # Initialize variables
    i = 0
    qubit_last_use = {}
    g = DAGCircuit()

    # Add the start node
    start_node = -1
    g.add_node(start_node, color="green")

    for gate in circuit.gates:
        # Transform gate to node
        hashable_gate = gate_to_node(gate, specific_label=i)
        i += 1
        g.add_node(hashable_gate, color="blue")

        # Add edges based on qubit_last_use; update last use
        for qubit in hashable_gate.pos:
            prev_node = qubit_last_use.get(qubit, start_node)
            g.add_edge(
                prev_node, hashable_gate, label=f'q{qubit}', color="green" if prev_node == start_node else "black")
            qubit_last_use[qubit] = hashable_gate

    if measure_flag:
        # Add measure_gate node
        measure_pos = copy.deepcopy(circuit.measures)
        measure_gate = InstructionNode("measure", measure_pos, None, None, None, label="m")
        g.add_node(measure_gate, color="blue")

        # Add edges from qubit_last_use to measure_gate
        for qubit in measure_gate.pos:
            prev_node = qubit_last_use.get(qubit, start_node)
            g.add_edge(
                prev_node, measure_gate, label=f'q{qubit}', color="green" if prev_node == start_node else "black")
            qubit_last_use[qubit] = measure_gate

    # Add the end node
    end_node = float('inf')
    g.add_node(end_node, color="red")

    for qubit, last_node in qubit_last_use.items():
        g.add_edge(last_node, end_node, label=f'q{qubit}', color="red")

    # Update DAGCircuit attributes
    g.update_qubits_used()
    g.update_cbits_used()
    g.update_num_instruction_nodes()
    g.update_circuit_qubits(circuit.num)

    return g


def node_to_gate(node_in_dag):
    """
    Transform a gate in a DAG graph to a gate in a circuit which can be added to a QuantumCircuit.

    Args:
        node_in_dag: A node in a DAG graph, an InstructionNode object. node_in_dag.name is uppercase,
                     node_in_dag.pos is a list or a dict. gate_transform supports gates with one or more qubits,
                     excluding measure nodes and special nodes [-1, float('inf'), measure_gate].

    Returns:
        gate: A gate which can be added to a QuantumCircuit in Quafu.

    Example:
        import networkx as nx
        from quafu import QuantumCircuit
        qcircuit = QuantumCircuit(n)

        for gate in nx.topological_sort(dep_graph):
            if gate not in [-1, float('inf')]:
                if gate.name == "measure":
                    qcircuit.measures = gate.pos
                else:
                    qcircuit.gates.append(node_to_gate(gate))
        return qcircuit
    """
    gate_name = node_in_dag.name.lower()
    gate_class = GATE_CLASSES.get(gate_name)

    if not gate_class:
        raise ValueError(f"Gate '{gate_name}' is not supported")

    if gate_name == "barrier":
        return gate_class(node_in_dag.pos)

    # Prepare arguments for gate initialization
    args = node_in_dag.pos
    if node_in_dag.paras:
        args += node_in_dag.paras

    # Handle specific gate types with additional parameters
    if gate_name in ["delay", "xy"]:
        args += [node_in_dag.duration, node_in_dag.unit]

    # Handle multi-qubit gates
    if gate_name in ["mcx", "mcy", "mcz"]:
        control_qubits = node_in_dag.pos[:-1]
        target_qubit = node_in_dag.pos[-1]
        return gate_class(control_qubits, target_qubit)

    return gate_class(*args)


def dag_to_circuit(dep_graph, qubits: int):
    """
    From DAG with Hashable Gates to quafu Gates added to circuit

    Args:
        dep_graph (DAG): DAG with Hashable Gates
        qubits (int): number of qubits

    Returns:
        qcircuit (QuantumCircuit): quafu QuantumCircuit

    Example:
        .. jupyter-execute::

            from circuit_dag import circuit_to_dag, dag_to_circuit, draw_dag
            from quafu import QuantumCircuit

            # Create a quantum circuit as an example that you can modify as needed
            circuit = QuantumCircuit(2)
            circuit.h(0)
            circuit.cnot(0, 1)

            # Build the dag graph
            dep_graph = circuit_to_dag(circuit)  # dag graph

            # Use dag_to_circuit to transform dag graph to a new circuit
            reconstructed_circuit = dag_to_circuit(dep_graph, circuit.num)
    """

    qcircuit = QuantumCircuit(qubits)
    for gate in nx.topological_sort(dep_graph):
        if gate not in [-1, float('inf')]:
            if gate.name == "measure":
                qcircuit.measures = gate.pos
            else:
                qcircuit.gates.append(node_to_gate(gate))
    return qcircuit


def draw_dag(dep_g, output_format="png"):
    """
    Helper function to visualize the DAG

    Args:
        dep_g (DAG): DAG with Hashable Gates
        output_format (str): output format, "png" or "svg"

    Returns:
        img (Image or SVG): show the image of DAG, which is Image(filename="dag.png") or SVG(filename="dag.svg")

    example:
        .. jupyter-execute::
        ex1:
            # directly draw  PNG picture
            draw_dag(dep_g, output_format="png")    # save a png picture "dag.png" and show it in jupyter notebook

            # directly draw  SVG   picture
            draw_dag(dep_g, output_format="svg")    # save a svg picture "dag.svg" and show it in jupyter notebook

        ex2:
            # generate   PNG  picture
            img_png = draw_dag(dep_g, output_format="png")

            # generate   SVG  picture
            img_svg = draw_dag(dep_g, output_format="svg")

            # show PNG picture
            img_png

            # show SVG picture
            img_svg
    """
    from IPython.display import Image, SVG

    try:
        import pygraphviz
        use_pygraphviz = True
    except ImportError:
        use_pygraphviz = False

    if use_pygraphviz:
        # Use pygraphviz
        from networkx.drawing.nx_agraph import write_dot
        write_dot(dep_g, "dag.dot")
        G = pygraphviz.AGraph("dag.dot")
        G.layout(prog="dot")

        if output_format == "png":
            G.draw("dag.png")
            return Image(filename="dag.png")
        elif output_format == "svg":
            G.draw("dag.svg")
            return SVG(filename="dag.svg")
        else:
            raise ValueError("Unsupported output format: choose either 'png' or 'svg'")
    else:
        try:
            # Use graphviz
            from graphviz import Digraph
        except ImportError:
            raise ImportError("Neither 'graphviz' nor 'pygraphviz' are installed.")

        dot = Digraph()

        # Add nodes and edges to the Digraph object based on the DAG
        for node in dep_g.nodes(data=True):
            dot.node(str(node[0]), color=node[1].get("color", "black"), style="solid")

        for edge in dep_g.edges(data=True):
            dot.edge(str(edge[0]), str(edge[1]), color=edge[2].get("color", "black"), label=edge[2].get("label", ""))

        # Render the graph to a file
        if output_format == "png":
            dot.format = 'png'
            dot.render("dag")
            return Image(filename="dag.png")
        elif output_format == "svg":
            dot.format = 'svg'
            dot.render("dag")
            return SVG(filename="dag.svg")
        else:
            raise ValueError("Unsupported output format: choose either 'png' or 'svg'")


def nodelist_to_dag(op_nodes: List[Any]) -> DAGCircuit:
    """
    Transform a list of operation nodes into a DAGCircuit.

    Args:
        op_nodes (List[Any]): A list of operation nodes.

    Returns:
        DAGCircuit: The constructed DAGCircuit object.
    """

    # Initialize variables
    qubit_last_use = {}
    g = DAGCircuit()

    # Add the start node
    start_node = -1
    g.add_node(start_node, color="green")

    for op_node in op_nodes:
        # Transform gate to node
        hashable_gate = copy.deepcopy(op_node)
        g.add_node(hashable_gate, color="blue")

        # Add edges based on qubit_last_use and update last use
        for qubit in hashable_gate.pos:
            prev_node = qubit_last_use.get(qubit, start_node)
            g.add_edge(
                prev_node, hashable_gate, label=f'q{qubit}', color="green" if prev_node == start_node else "black")
            qubit_last_use[qubit] = hashable_gate

    # Add the end node
    end_node = float('inf')
    g.add_node(end_node, color="red")

    for qubit, last_node in qubit_last_use.items():
        g.add_edge(last_node, end_node, label=f'q{qubit}', color="red")

    # Update DAGCircuit attributes
    g.qubits_used = g.update_qubits_used()
    g.cbits_used = g.update_cbits_used()
    g.num_instruction_nodes = g.update_num_instruction_nodes()

    return g


def nodelist_qubit_mapping_dict(nodes_list):
    """
    Generate a mapping dictionary for qubits used by the nodes to new qubits.

    Args:
        nodes_list: a list of nodes

    Returns:
        nodes_qubit_mapping_dict: a dict where keys are the qubits used by the nodes and values are the new qubits
    """
    nodes_list_qubits_used = set()
    for node in nodes_list:
        if hasattr(node, 'pos') and node.pos is not None:
            nodes_list_qubits_used.update(node.pos)

    mapping_pos = list(range(len(nodes_list_qubits_used)))
    # Create the mapping dictionary
    nodes_qubit_mapping_dict = dict(zip(sorted(nodes_list_qubits_used), mapping_pos))

    return nodes_qubit_mapping_dict


def nodelist_qubit_mapping_dict_reverse(nodes_list):
    """
    Generate a reverse mapping dictionary for qubits used by the nodes.

    Args:
        nodes_list: a list of nodes

    Returns:
        nodes_qubit_mapping_dict_reverse: a dict where keys are the new qubits and values are the qubits used by the nodes
    """
    # Generate the initial mapping dictionary
    nodes_qubit_mapping_dict = nodelist_qubit_mapping_dict(nodes_list)

    # Reverse the mapping
    nodes_qubit_mapping_dict_reverse = {value: key for key, value in nodes_qubit_mapping_dict.items()}

    return nodes_qubit_mapping_dict_reverse


def nodes_list_mapping(nodes_list, nodes_qubit_mapping_dict):
    """
    Map qubits in nodes_list to new qubits using the provided mapping dictionary.

    Args:
        nodes_list: The list of instruction nodes.
        nodes_qubit_mapping_dict: The dictionary mapping old qubits to new qubits.

    Returns:
        nodes_list_mapping: The nodes_list after mapping qubits.
    """
    nodes_mapping = []
    for node in nodes_list:
        node_new = copy.deepcopy(node)
        if hasattr(node, 'pos') and node.pos is not None:
            if isinstance(node.pos, list):
                node_new.pos = [nodes_qubit_mapping_dict[qubit] for qubit in node.pos]
            elif isinstance(node.pos, dict):
                node_new.pos = {}
                # The values of the dict are void, so we need to copy the values from the original dict
                for qubit in node.pos:
                    node_new.pos[nodes_qubit_mapping_dict[qubit]] = copy.deepcopy(node.pos[qubit])
        nodes_mapping.append(node_new)
    return nodes_mapping


def copy_dag(dag: DAGCircuit):
    """
    Copy a DAGCircuit object.

    Args:
        dag (DAGCircuit): The DAGCircuit to be copied.

    Returns:
        dag_copy (DAGCircuit): The copied DAGCircuit.
    """
    dag_copy = DAGCircuit()
    dag_copy.circuit_qubits = copy.deepcopy(dag.circuit_qubits)
    dag_copy.qubits_used = copy.deepcopy(dag.qubits_used)
    dag_copy.cbits_used = copy.deepcopy(dag.cbits_used)
    dag_copy.num_instruction_nodes = copy.deepcopy(dag.num_instruction_nodes)
    dag_copy.add_weighted_edges_from(copy.deepcopy(dag.edges(data=True)))
    return dag_copy


def show_dag(dag: DAGCircuit) -> None:
    """
    Show the DAG of a circuit in a figure with proper size in interactive mode.
    This function is used for debugging such as in PyCharm and other IDEs.

    Args:
        dag (DAGCircuit): The DAGCircuit to be displayed.
        dpi (int): Dots per inch, to specify the resolution of the image. Default is 200.
    """
    import matplotlib.pyplot as plt
    from PIL import Image

    draw_dag(dag)
    im = Image.open('dag.png')

    # Convert to array and plot
    data = np.array(im)
    original_width, original_height = data.shape[1], data.shape[0]

    # Display the image with appropriate size and resolution
    dpi = 100
    plt.figure(figsize=(original_width / dpi, original_height / dpi), dpi=dpi)
    plt.imshow(data)
    plt.axis('off')
    plt.subplots_adjust(left=0, right=1, top=1, bottom=0)
    plt.show()
