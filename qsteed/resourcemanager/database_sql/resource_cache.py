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

from collections import defaultdict
from dataclasses import dataclass
from threading import RLock
from types import MappingProxyType
from typing import Callable, Mapping, Optional, Sequence, Tuple


def normalize_qpu_name(name: Optional[str]) -> Optional[str]:
    """Return the case-insensitive cache key for a QPU name."""
    if not isinstance(name, str):
        return None
    normalized = name.strip().casefold()
    return normalized or None


def _coupling_qubits(coupling_list) -> frozenset:
    if not coupling_list:
        return frozenset()
    return frozenset(
        qubit
        for coupling in coupling_list
        for qubit in coupling[:2]
    )


def _named_rows(rows: Sequence) -> Tuple:
    """Drop legacy empty rows before they enter the runtime cache."""
    return tuple(
        row for row in rows
        if normalize_qpu_name(getattr(row, 'qpu_name', None)) is not None
    )


def _freeze_grouped_index(index):
    return MappingProxyType({key: tuple(value) for key, value in index.items()})


@dataclass(frozen=True)
class ResourceSnapshot:
    """Immutable collection of one consistent version of cached resources."""
    revision: int
    qpus: Tuple
    stdqpus: Tuple
    subqpus: Tuple
    vqpus: Tuple
    qpu_by_name: Mapping
    stdqpu_by_name: Mapping
    subqpus_by_size: Mapping
    subqpus_by_key: Mapping
    subqpus_by_qubits: Mapping
    vqpus_by_size: Mapping
    vqpus_by_key: Mapping
    vqpus_by_qubits: Mapping

    def get_qpu(self, qpu_name: str):
        return self.qpu_by_name.get(normalize_qpu_name(qpu_name))

    def get_stdqpu(self, qpu_name: str):
        return self.stdqpu_by_name.get(normalize_qpu_name(qpu_name))

    def get_subqpus(self, qpu_name: str = None, qubits_num: int = None):
        if qubits_num is None:
            raise ValueError("The qubits_num is not specified.")
        if qpu_name is None:
            return self.subqpus_by_size.get(qubits_num, ())
        key = (normalize_qpu_name(qpu_name), qubits_num)
        return self.subqpus_by_key.get(key, ())

    def get_vqpus(self, qpu_name: str = None, qubits_num: int = None):
        if qubits_num is None:
            raise ValueError("The qubits_num is not specified.")
        if qpu_name is None:
            return self.vqpus_by_size.get(qubits_num, ())
        key = (normalize_qpu_name(qpu_name), qubits_num)
        return self.vqpus_by_key.get(key, ())

    def get_subqpus_for_qubits(self, qpu_name: str, qubits_list):
        key = (normalize_qpu_name(qpu_name), frozenset(qubits_list))
        return self.subqpus_by_qubits.get(key, ())

    def get_vqpus_for_qubits(self, qpu_name: str, qubits_list):
        key = (normalize_qpu_name(qpu_name), frozenset(qubits_list))
        return self.vqpus_by_qubits.get(key, ())


def build_resource_snapshot(qpus=(), stdqpus=(), subqpus=(), vqpus=(), revision=0):
    """Build an indexed snapshot without exposing legacy empty rows."""
    qpus = _named_rows(qpus)
    stdqpus = _named_rows(stdqpus)
    subqpus = _named_rows(subqpus)
    vqpus = _named_rows(vqpus)

    qpu_by_name = {}
    for qpu in qpus:
        qpu_by_name.setdefault(normalize_qpu_name(qpu.qpu_name), qpu)

    stdqpu_by_name = {}
    for stdqpu in stdqpus:
        stdqpu_by_name.setdefault(normalize_qpu_name(stdqpu.qpu_name), stdqpu)

    subqpus_by_size = defaultdict(list)
    subqpus_by_key = defaultdict(list)
    subqpus_by_qubits = defaultdict(list)
    for subqpu in subqpus:
        name = normalize_qpu_name(subqpu.qpu_name)
        subqpus_by_size[subqpu.qubits_num].append(subqpu)
        subqpus_by_key[(name, subqpu.qubits_num)].append(subqpu)
        qubits = _coupling_qubits(getattr(subqpu, 'substructure_CAL', None))
        subqpus_by_qubits[(name, qubits)].append(subqpu)

    vqpus_by_size = defaultdict(list)
    vqpus_by_key = defaultdict(list)
    vqpus_by_qubits = defaultdict(list)
    for vqpu in vqpus:
        name = normalize_qpu_name(vqpu.qpu_name)
        vqpus_by_size[vqpu.qubits_num].append(vqpu)
        vqpus_by_key[(name, vqpu.qubits_num)].append(vqpu)
        qubits = _coupling_qubits(getattr(vqpu, 'coupling_list', None))
        vqpus_by_qubits[(name, qubits)].append(vqpu)

    return ResourceSnapshot(
        revision=revision,
        qpus=qpus,
        stdqpus=stdqpus,
        subqpus=subqpus,
        vqpus=vqpus,
        qpu_by_name=MappingProxyType(qpu_by_name),
        stdqpu_by_name=MappingProxyType(stdqpu_by_name),
        subqpus_by_size=_freeze_grouped_index(subqpus_by_size),
        subqpus_by_key=_freeze_grouped_index(subqpus_by_key),
        subqpus_by_qubits=_freeze_grouped_index(subqpus_by_qubits),
        vqpus_by_size=_freeze_grouped_index(vqpus_by_size),
        vqpus_by_key=_freeze_grouped_index(vqpus_by_key),
        vqpus_by_qubits=_freeze_grouped_index(vqpus_by_qubits),
    )


class ResourceCache:
    """Thread-safe loader that atomically replaces complete snapshots."""

    def __init__(self, loader: Callable[[], tuple]):
        self._loader = loader
        self._refresh_lock = RLock()
        self._snapshot = None

    def current(self) -> ResourceSnapshot:
        snapshot = self._snapshot
        if snapshot is not None:
            return snapshot
        with self._refresh_lock:
            if self._snapshot is None:
                self._snapshot = self._load_snapshot()
            return self._snapshot

    def refresh(self) -> ResourceSnapshot:
        with self._refresh_lock:
            snapshot = self._load_snapshot()
            # A single reference assignment keeps readers on either the old or
            # the new complete snapshot; they never observe a partial refresh.
            self._snapshot = snapshot
            return snapshot

    def _load_snapshot(self) -> ResourceSnapshot:
        qpus, stdqpus, subqpus, vqpus = self._loader()
        current_revision = self._snapshot.revision if self._snapshot else 0
        return build_resource_snapshot(
            qpus=qpus,
            stdqpus=stdqpus,
            subqpus=subqpus,
            vqpus=vqpus,
            revision=current_revision + 1,
        )
