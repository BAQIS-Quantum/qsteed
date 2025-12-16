# qsteed/utils/gates.py

from .. import qsteedcpp

_ALL_FACTORIES = [
    # Single-qubit gates
    qsteedcpp.H,
    qsteedcpp.X,
    qsteedcpp.Y,
    qsteedcpp.Z,
    qsteedcpp.S,
    qsteedcpp.Sdg,
    qsteedcpp.T,
    qsteedcpp.Tdg,
    qsteedcpp.Id,
    qsteedcpp.SX,
    qsteedcpp.SXdg,
    qsteedcpp.SY,
    qsteedcpp.SYdg,
    qsteedcpp.W,
    qsteedcpp.SW,
    qsteedcpp.SWdg,
    qsteedcpp.RX,
    qsteedcpp.RY,
    qsteedcpp.RZ,
    qsteedcpp.Phase, # P's alias
    qsteedcpp.U3,
    
    # Two-qubit gates
    qsteedcpp.CNOT,
    qsteedcpp.CX,    # CNOT's alias
    qsteedcpp.CZ,
    qsteedcpp.CY,
    qsteedcpp.CS,
    qsteedcpp.CT,
    qsteedcpp.Swap,
    qsteedcpp.ISwap,
    qsteedcpp.RXX,
    qsteedcpp.RYY,
    qsteedcpp.RZZ,
    qsteedcpp.CP,
    qsteedcpp.CRX,
    qsteedcpp.CRY,
    qsteedcpp.CRZ,

    # Three-qubit gates
    qsteedcpp.Toffoli,
    qsteedcpp.CCX,   # Toffoli's alias
    qsteedcpp.Fredkin,
    qsteedcpp.CSWAP, # Fredkin's alias
    
    # Multi-controlled gates
    qsteedcpp.MCX,
    qsteedcpp.MCY,
    qsteedcpp.MCZ,
    qsteedcpp.MCRX,
    qsteedcpp.MCRY,
    qsteedcpp.MCRZ,
    qsteedcpp.ControlledU,

    # Non-gate operations
    qsteedcpp.Measure,
    qsteedcpp.Barrier,
    qsteedcpp.Reset,
    qsteedcpp.Delay,
    qsteedcpp.XYResonance,
]


gate_classes = {}
gate_metadata = {}

for factory_instance in _ALL_FACTORIES:
    gate_name_str = factory_instance.name.lower()
    gate_classes[gate_name_str] = factory_instance


    if gate_name_str in ['h', 'x', 'y', 'z', 's', 'sdg', 't', 'tdg', 'id', 'sx', 'sxdg', 'sy', 'sydg', 'w', 'sw', 'swdg']:
        gate_metadata[gate_name_str] = {"num_qubits": 1, "num_params": 0, "category": "1q-nonparam"}
    elif gate_name_str in ['rx', 'ry', 'rz', 'p', 'phase']:
        gate_metadata[gate_name_str] = {"num_qubits": 1, "num_params": 1, "category": "1q-param"}
    elif gate_name_str == 'u3':
        gate_metadata[gate_name_str] = {"num_qubits": 1, "num_params": 3, "category": "1q-param"}
    elif gate_name_str in ['cnot', 'cx', 'cz', 'cy', 'cs', 'ct', 'swap', 'iswap']:
        gate_metadata[gate_name_str] = {"num_qubits": 2, "num_params": 0, "category": "2q-nonparam"}
    elif gate_name_str in ['rxx', 'ryy', 'rzz', 'cp', 'crx', 'cry', 'crz']:
        gate_metadata[gate_name_str] = {"num_qubits": 2, "num_params": 1, "category": "2q-param"}
    elif gate_name_str in ['toffoli', 'ccx', 'fredkin', 'cswap']:
        gate_metadata[gate_name_str] = {"num_qubits": 3, "num_params": 0, "category": "3q-nonparam"}
    elif gate_name_str in ['mcx', 'mcy', 'mcz', 'controlledu']:
        gate_metadata[gate_name_str] = {"num_qubits": -1, "num_params": 0, "category": "mc-nonparam"}
    elif gate_name_str in ['mcrx', 'mcry', 'mcrz']:
        gate_metadata[gate_name_str] = {"num_qubits": -1, "num_params": 1, "category": "mc-param"}
    else: # non-gate
        gate_metadata[gate_name_str] = {"num_qubits": -1, "num_params": -1, "category": "non-gate"}


SUPPORTED_GATES = list(gate_classes.keys())


__all__ = ["gate_classes", "gate_metadata", "SUPPORTED_GATES"]
