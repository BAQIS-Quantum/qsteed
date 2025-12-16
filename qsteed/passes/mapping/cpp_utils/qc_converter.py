from qsteed.qsteedcpp import DAGCircuit as Cpp_DAGCircuit
from qsteed.qsteedcpp import InstructionNode as Cpp_InstructionNode
from qsteed.qsteedcpp import MeasureNode as Cpp_MeasureNode
from qsteed.qsteedcpp import QuantumCircuit, CircuitInstruction
from qsteed.qsteedcpp import Expr
from qsteed.utils.gates import gate_classes as GATE_CLASSES

def QuantumCircuit_to_cppDag(circuit: QuantumCircuit) -> Cpp_DAGCircuit:
    """
    Converts a qsteed QuantumCircuit object to a C++ Cpp_DAGCircuit object.
    """
    dag = Cpp_DAGCircuit()

    used_qubits = set()
    for gate in circuit.instructions:
        node = gate_to_cppNode(gate)
        dag.add_instruction_node_end(node)
        for pos in node.qubit_pos:
            used_qubits.add(pos)

    if circuit.measures:
        dag.measure = circuit.measures

    return dag


def cppDag_to_QuantumCircuit(dag: Cpp_DAGCircuit, num_qubits: int) -> QuantumCircuit:
    """
    Converts a Cpp_DAGCircuit to a qsteed QuantumCircuit.
    """
    q_circuit = QuantumCircuit(num_qubits)
    for vertex in dag.vertices():
        if dag.graph[vertex].name in ["start", "end"]:
            continue
        else:
            gate_instruction = cppNode_to_gate(dag.graph[vertex])
            q_circuit.append(gate_instruction)

    if dag.measure:
        q_circuit.measure(list(dag.measure.keys()), list(dag.measure.values()))

    return q_circuit


def gate_to_cppNode(gate: CircuitInstruction) -> Cpp_InstructionNode:
    """
    Converts a qsteed CircuitInstruction to a C++ Cpp_InstructionNode.
    """
    paras = gate.paras if gate.paras is not None else []

    if gate.name.lower() == "measure":
        node = Cpp_MeasureNode(gate.qubits, gate.clbits)
        return node

    if gate.duration is not None and gate.unit is not None:
        node = Cpp_InstructionNode(gate.name, gate.qubits, paras, gate.duration, gate.unit)
    else:
        if paras:
            node = Cpp_InstructionNode(gate.name, gate.qubits, paras, 0, "")
        else:
            node = Cpp_InstructionNode(gate.name, gate.qubits)

    return node


def cppNode_to_gate(node: Cpp_InstructionNode) -> CircuitInstruction:
    """
    Converts a Cpp_InstructionNode back to a qsteed CircuitInstruction.
    """
    gate_name = node.name.lower()
    gate_class = GATE_CLASSES.get(gate_name)

    if not gate_class:
        raise ValueError(f"Gate '{gate_name}' is not supported for conversion from Cpp_InstructionNode")

    if gate_name == "measure":
        return gate_class(node.qubit_pos, node.classic_pos)

    param_first_gates = ["rx", "ry", "rz", "p", "phase", "rxx", "ryy", "rzz",
                         "cp", "crx", "cry", "crz", "mcrx", "mcry", "mcrz", "u3"]

    if gate_name in param_first_gates:
        args = node.paras + node.qubit_pos
    else:
        args = node.qubit_pos + node.paras

    # Handle special signatures
    if gate_name in ["delay", "xy"]:
        args = node.qubit_pos + [node.duration, node.unit]
    elif gate_name in ["mcx", "mcy", "mcz"]:
        control_qubits = node.qubit_pos[:-1]
        target_qubit = node.qubit_pos[-1]
        return gate_class(control_qubits, target_qubit)
    elif gate_name == "barrier":
        return gate_class(node.qubit_pos)

    return gate_class(*args) 