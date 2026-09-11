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

from threading import Event, Thread
from types import SimpleNamespace
from unittest.mock import patch

from qsteed.compiler.compiler import Compiler
from qsteed.resourcemanager.database_sql.resource_cache import (
    ResourceCache,
    build_resource_snapshot,
)


def _resource(qpu_name, qubits_num=None, coupling_list=None, substructure_CAL=None):
    return SimpleNamespace(
        qpu_name=qpu_name,
        qubits_num=qubits_num,
        coupling_list=coupling_list,
        substructure_CAL=substructure_CAL,
    )


def test_snapshot_filters_empty_rows_and_builds_case_insensitive_indexes():
    empty = _resource(None)
    qpu = _resource('Dongling')
    stdqpu = _resource('Dongling')
    vqpu = _resource('Dongling', 3, [(1, 2, 0.99), (2, 3, 0.98)])
    subqpu = _resource(
        'Dongling',
        3,
        substructure_CAL=[(1, 2, 0.99), (2, 3, 0.98)],
    )

    snapshot = build_resource_snapshot(
        qpus=[empty, qpu],
        stdqpus=[empty, stdqpu],
        subqpus=[empty, subqpu],
        vqpus=[empty, vqpu],
        revision=7,
    )

    assert snapshot.revision == 7
    assert snapshot.qpus == (qpu,)
    assert snapshot.get_qpu('  DONGLING ') is qpu
    assert snapshot.get_stdqpu('dongling') is stdqpu
    assert snapshot.get_vqpus('dongling', 3) == (vqpu,)
    assert snapshot.get_subqpus('DONGLING', 3) == (subqpu,)
    assert snapshot.get_vqpus_for_qubits('Dongling', [3, 2, 1]) == (vqpu,)
    assert snapshot.get_subqpus_for_qubits('dongling', [1, 2, 3]) == (subqpu,)


def test_refresh_atomically_replaces_the_complete_snapshot():
    refresh_started = Event()
    allow_refresh = Event()
    old_qpu = _resource('Old')
    new_qpu = _resource('New')
    load_count = 0

    def loader():
        nonlocal load_count
        load_count += 1
        if load_count == 1:
            return [old_qpu], [], [], []
        refresh_started.set()
        assert allow_refresh.wait(timeout=2)
        return [new_qpu], [], [], []

    cache = ResourceCache(loader)
    old_snapshot = cache.current()
    refresh_thread = Thread(target=cache.refresh)
    refresh_thread.start()

    assert refresh_started.wait(timeout=2)
    assert cache.current() is old_snapshot
    assert cache.current().get_qpu('old') is old_qpu

    allow_refresh.set()
    refresh_thread.join(timeout=2)
    assert not refresh_thread.is_alive()

    new_snapshot = cache.current()
    assert new_snapshot.revision == old_snapshot.revision + 1
    assert new_snapshot.get_qpu('new') is new_qpu
    assert old_snapshot.get_qpu('old') is old_qpu


def test_failed_refresh_keeps_the_previous_snapshot():
    old_qpu = _resource('Old')
    should_fail = False

    def loader():
        if should_fail:
            raise RuntimeError('database unavailable')
        return [old_qpu], [], [], []

    cache = ResourceCache(loader)
    old_snapshot = cache.current()
    should_fail = True

    try:
        cache.refresh()
    except RuntimeError:
        pass
    else:
        raise AssertionError('refresh should propagate loader errors')

    assert cache.current() is old_snapshot


def test_compiler_pins_one_snapshot_for_each_compile_call():
    first_snapshot = build_resource_snapshot(revision=1)
    second_snapshot = build_resource_snapshot(revision=2)
    compiler = object.__new__(Compiler)
    compiler._resource_snapshot = None
    compiler.circuit = None

    with patch(
        'qsteed.compiler.compiler.get_resource_snapshot',
        side_effect=[first_snapshot, second_snapshot],
    ) as get_snapshot:
        assert compiler._get_resource_snapshot() is first_snapshot
        assert compiler._get_resource_snapshot() is first_snapshot

        try:
            compiler.compile()
        except TypeError:
            pass
        else:
            raise AssertionError('the invalid test circuit should not compile')

        assert compiler._resource_snapshot is second_snapshot
        assert get_snapshot.call_count == 2
