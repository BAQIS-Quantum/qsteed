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


from qsteed.resourcemanager.database_sql.sql_models import QPU, SubQPU, VQPU, StdQPU
from qsteed.resourcemanager.database_sql.initialize_app_db import app
from qsteed.resourcemanager.database_sql.resource_cache import ResourceCache


def _load_resources():
    with app.app_context():
        return (
            QPU.query.all(),
            StdQPU.query.all(),
            SubQPU.query.all(),
            VQPU.query.all(),
        )


_RESOURCE_CACHE = ResourceCache(_load_resources)


def get_resource_snapshot():
    """Return the current complete in-memory resource snapshot."""
    return _RESOURCE_CACHE.current()


def get_qpu():
    return list(get_resource_snapshot().qpus)


def get_stdqpu():
    return list(get_resource_snapshot().stdqpus)


def get_subqpu():
    return list(get_resource_snapshot().subqpus)


def get_vqpu():
    return list(get_resource_snapshot().vqpus)


def instantiating_qpu(_app=None):
    return get_qpu()


def instantiating_stdqpu(_app=None):
    return get_stdqpu()


def instantiating_subqpu(_app=None):
    return get_subqpu()


def instantiating_vqpu(_app=None):
    return get_vqpu()


def update_memory_database():
    """Reload all resource tables and atomically publish one new snapshot."""
    snapshot = _RESOURCE_CACHE.refresh()
    return (
        list(snapshot.qpus),
        list(snapshot.stdqpus),
        list(snapshot.subqpus),
        list(snapshot.vqpus),
    )
