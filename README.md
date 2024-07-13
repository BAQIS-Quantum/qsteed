# QSteed

[![License](https://img.shields.io/github/license/BAQIS-Quantum/qsteed.svg?)](https://opensource.org/licenses/Apache-2.0)
[![Current Release](https://img.shields.io/github/release/BAQIS-Quantum/qsteed.svg?)](https://github.com/BAQIS-Quantum/qsteed/releases)
![Python versions](https://img.shields.io/badge/python-3.10%20%7C%203.11%20%7C%203.12-blue)
[![Downloads](https://img.shields.io/pypi/dm/qsteed.svg?)](https://pypi.org/project/qsteed/)

[//]: # (![PyPI - Python Version]&#40;https://img.shields.io/pypi/pyversions/qsteed&#41;)


**QSteed** is a **Q**uantum **S**of**t**war**e** of 
Compilation for supporting R**e**al Quantum **d**evice, 
including a quantum compiler, a quantum resource 
virtualization manager, and a task scheduler.

## Installation
### Install from PyPI
You can install QSteed via pip:
```bash
pip install qsteed
```

### Install from source
You can directly download the [source code](https://github.com/BAQIS-Quantum/qsteed/archive/refs/heads/master.zip) from GitHub
or clone the repository using the following command.
```bash
git clone https://github.com/BAQIS-Quantum/QSteed.git
```
Change to the qsteed directory install using the following command.
```bash
pip install -r requirements.txt
python setup.py install
```

## Example
```python
import matplotlib.pyplot as plt
from qsteed import Transpiler
from qsteed.backends.backend import Backend
from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
from qsteed.passes.model import Model
from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
from qsteed.passes.optimization.one_qubit_optimization import OneQubitGateOptimization
from qsteed.passes.unroll.unroll_to_2qubit import UnrollTo2Qubit
from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis
from qsteed.utils.random_circuit import RandomCircuit
from qsteed.passflow.passflow import PassFlow

# Generating random quantum circuits (needs to be a pyquafu QuantumCircuit class)
rqc = RandomCircuit(num_qubit=5, gates_number=100, gates_list=['cx', 'rx', 'rz', 'ry', 'h'])
qc = rqc.random_circuit()

# Set chip information (the number of chip qubits needs to be consistent
# with the number of quantum circuit qubits)
basis_gates = ['cx', 'rx', 'ry', 'rz', 'id', 'h']
c_list = [(2, 3, 0.982), (3, 2, 0.982), (3, 4, 0.973), (4, 3, 0.973), 
          (0, 1, 0.98), (1, 0, 0.98), (1, 2, 0.97), (2, 1, 0.97)]
backend_properties = {
    'name': 'ExampleBackend',
    'backend_type': 'superconducting',
    'qubits_num': 5,
    'coupling_list': c_list,
    'basis_gates': basis_gates,
}

# Predefined compilation passflow
passes = [
    UnrollTo2Qubit(),
    SabreLayout(heuristic='fidelity', max_iterations=3),  # heuristic='distance' or 'fidelity', 'mixture'
    UnrollToBasis(basis_gates=basis_gates),
    GateCombineOptimization(),
    OneQubitGateOptimization()
]
passflow = PassFlow(passes=passes)

backend_instance = Backend(**backend_properties)
initial_model = Model(backend=backend_instance)

compiler = Transpiler(passflow, initial_model)
compiled_circuit = compiler.transpile(qc)
compiled_circuit.plot_circuit()
plt.show()
```

You can also use preset compilation passflow with optimization_level 0-3:
Using preset compilation passflow, see [preset_passflow.py](qsteed/passflow/preset_passflow.py)
```python
compiler = Transpiler(initial_model=initial_model)
compiled_circuit = compiler.transpile(qc, optimization_level=3)
```

## More Tests
See [tests](tests) for more examples.

## Real machine deployment
QSteed has been successfully deployed to the [Quafu Quantum Cloud Computing Cluster](https://quafu.baqis.ac.cn/), 
supporting the compilation and optimization of user tasks into quantum circuits 
executable by quantum processors. 
You can submit tasks through the [Quafu Composer](https://quafu.baqis.ac.cn/#/composer) web interface,
[Pyquafu](https://scq-cloud.github.io/) client, 
or [QuarkStudio](https://www.yuque.com/wuming-6g8w2/ghzgfk/azika5xqlpxig3q3?singleDoc#) client.

The public version of QSteed does not yet provide features such as 
quantum resource management and unified software-hardware interface calls. 
We will gradually make these features available according to our plan. 
If you would like to learn more about real machine deployment, please contact us.


## How to contribute
For information on how to contribute, please send an e-mail to members of developer of this project.

QSteed was developed by the quantum operating system team of the Beijing Academy of Quantum Information Sciences.

## License
QSteed is released under the Apache 2.0 license. See [LICENSE](LICENSE) for more details.
