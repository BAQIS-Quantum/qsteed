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


from ast import literal_eval
from typing import Any, Dict, Mapping, Tuple


def qlabel_from_xy(x: int, y: int) -> str:
    """Generate a QSteed qubit label from grid coordinates."""
    return f"Q{y + 1:02d}{x + 1:02d}"


def index_to_xy(qubits_info: Mapping[str, Mapping[str, Any]]) -> Dict[int, Tuple[int, int]]:
    """Extract an index-to-coordinate mapping from qubit metadata."""
    idx2xy = {}

    for qv in qubits_info.values():
        index = qv.get("index")
        coordinate = qv.get("coordinate", [None, None])
        if index is None or not isinstance(coordinate, (list, tuple)) or len(coordinate) < 2:
            continue

        x, y = coordinate[:2]
        if x is None or y is None:
            continue

        idx2xy[int(index)] = (int(x), int(y))

    return idx2xy


def index_to_qlabel(qubits_info: Mapping[str, Mapping[str, Any]]) -> Dict[int, str]:
    """Convert qubit indexes to QSteed qubit labels."""
    idx2xy = index_to_xy(qubits_info)
    return {index: qlabel_from_xy(x, y) for index, (x, y) in idx2xy.items()}


def _as_float(value: Any, default: float = 0.0) -> float:
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def _normalize_basis_gates(basis_gates: Any) -> list:
    if basis_gates is None:
        return []

    if isinstance(basis_gates, str):
        try:
            parsed = literal_eval(basis_gates)
        except (SyntaxError, ValueError):
            parsed = [basis_gates]
        basis_gates = parsed if isinstance(parsed, (list, tuple, set)) else [parsed]

    return [str(gate).lower() for gate in basis_gates]


def transform_quafu_chip(info: Mapping[str, Any], keep_priority: bool = True) -> Dict[str, Any]:
    """Transform raw Quafu cloud chip information into QSteed resource DB format.

    The returned dictionary can be passed to ``update_chip_api`` after setting
    the ``system_id`` that matches the user's QSteed config.
    """
    calibration_time = info.get("calibration_time", "")
    basis_gates = str(_normalize_basis_gates(info.get("basis_gates", [])))

    raw_qubits_info = info.get("qubits_info", {})
    idx2qlabel = index_to_qlabel(raw_qubits_info)

    all_qubits_info = {qlabel: {} for qlabel in idx2qlabel.values()}
    qubits_info = {}

    for qv in raw_qubits_info.values():
        index = qv.get("index")
        if index is None or int(index) not in idx2qlabel:
            continue

        qlabel = idx2qlabel[int(index)]
        frequency = qv.get("frequency", 0.0)
        qubit_frequency = round(_as_float(frequency) * 1e9, 3)

        qubits_info[qlabel] = {
            "T1": round(_as_float(qv.get("T1", 0.0)), 3),
            "T2": round(_as_float(qv.get("T2", 0.0)), 3),
            "anharmonicity": _as_float(qv.get("anharmonicity", 0.0)),
            "qubit_frequency": qubit_frequency,
            "readout_frequency": _as_float(qv.get("readout_frequency", 0.0)),
            "single_qubit_gate_duration": _as_float(qv.get("single_qubit_gate_duration", 0.0)),
            "single_qubit_gate_fidelity": round(_as_float(qv.get("fidelity", 0.0)), 5),
        }

    if keep_priority:
        priority_qubits = str(info.get("priority_qubits", []))
    else:
        priority_qubits = "[[]]"

    global_info = info.get("global_info", {})
    two_qubit_gate = global_info.get("two_qubit_gate_basis", "CZ")
    topological_structure = {}

    for cv in info.get("couplers_info", {}).values():
        qubits_index = cv.get("qubits_index", [None, None])
        if not isinstance(qubits_index, (list, tuple)) or len(qubits_index) < 2:
            continue

        first, second = qubits_index[:2]
        first_label = idx2qlabel.get(first)
        second_label = idx2qlabel.get(second)
        if not first_label or not second_label:
            continue

        qubit_a, qubit_b = sorted([first_label, second_label])
        topological_structure[f"{qubit_a}_{qubit_b}"] = {
            two_qubit_gate: {
                "fidelity": round(_as_float(cv.get("fidelity", 0.0)), 3)
            }
        }

    return {
        "calibration_time": calibration_time,
        "basis_gates": basis_gates,
        "priority_qubits": priority_qubits,
        "all_qubits_info": all_qubits_info,
        "qubits_info": qubits_info,
        "all_topological_structure": topological_structure,
        "topological_structure": topological_structure,
    }
