# QSteed

[![License](https://img.shields.io/github/license/BAQIS-Quantum/qsteed.svg?)](https://opensource.org/licenses/Apache-2.0)
[![Current Release](https://img.shields.io/github/release/BAQIS-Quantum/qsteed.svg?)](https://github.com/BAQIS-Quantum/qsteed/releases)
![Python versions](https://img.shields.io/badge/python-3.10%20%7C%203.11-blue)
[![Downloads](https://img.shields.io/pypi/dm/qsteed.svg?)](https://pypi.org/project/qsteed/)

[//]: # (![PyPI - Python Version]&#40;https://img.shields.io/pypi/pyversions/qsteed&#41;)


**QSteed** is a **Q**uantum **S**of**t**war**e** of 
Compilation for supporting R**e**al Quantum **d**evice, 
including a quantum compiler, a quantum resource 
virtualization manager, and a task scheduler.

## Installation

### Need to install [pyquafu](https://github.com/ScQ-Cloud/pyquafu)
```bash
pip install 'pyquafu>=0.4.1'
```
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
Change to the qsteed directory install using the following command:
```bash
pip install -r requirements.txt
python setup.py install   # or: pip install .
```

## Example
### Quantum circuit transpiler
To use only the quantum circuit transpiler, you can refer to the following examples.
The following code demonstrates how to customize hardware backend properties and customize the compilation process.
```python
import matplotlib.pyplot as plt
from qsteed import *

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

transpiler = Transpiler(passflow, initial_model)
transpiled_circuit = transpiler.transpile(qc)
transpiled_circuit.plot_circuit()
plt.show()
```

You can also use preset compilation passflow with optimization_level 0-3:
Using preset compilation passflow, see [preset_passflow.py](qsteed/passflow/preset_passflow.py)
```python
transpiler = Transpiler(initial_model=initial_model)
transpiled_circuit = transpiler.transpile(qc, optimization_level=3)
```

### Quantum Compiler
> ⚠️<span style="color:#8B0000"> **Warning**</span>   
> The quantum compiler requires MySQL database support, see section [Deployment](#deployment).

Using the `Compiler`, you can compile quantum circuits onto a real quantum chip.
```python
from qsteed.compiler.compiler import Compiler

qasm = """
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg meas[5];
rxx(2.7757800154614016) q[3],q[2];
z q[2];
h q[3];
rxx(5.893149917736792) q[2],q[0];
cx q[4],q[1];
x q[1];
y q[4];
x q[4];
measure q[0] -> meas[0];
measure q[1] -> meas[1];
measure q[2] -> meas[2];
measure q[3] -> meas[3];
measure q[4] -> meas[4];
"""

# If 'qpu_name' is not given, the most suitable computing resource for the task is searched on all available chips. 
compiler = Compiler(qasm, qpu_name='example')
compiled_openqasm, final_q2c, compiled_circuit_information = compiler.compile()
```

More convenient to use `compiler_api`, user tasks can be compiled onto available quantum computing resources.
If deployed on a real machine, users can submit a task information dictionary, 
and by invoking the compilation interface, the compiled results will be sent to 
the quantum computer’s measurement and control device for computation.
```python
from qsteed.apis.compiler_api import call_compiler_api

# Assume you can obtain the user's task information and store it as task_info. 
task_info = {
    "circuit": qasm,
    "transpile": True,
    "qpu_name": 'example',
    "optimization_level": 2,
    "task_type": 'qc',
}
compiled_info = call_compiler_api(**task_info)
print('Compiled openqasm:\n', compiled_info[0])
print('Measurement qubits to cbits:\n', compiled_info[1])
print('Compiled circuit information:\n', compiled_info[2])
```


## Deployment
### Copy the configuration file
For convenience, you can run the following command to place the configuration file
`config.ini` in the `QSteed` folder at the root directory, or you can create it manually.
```python
from qsteed.qsteed_config import copy_config
copy_config()
```
or run the following command in the terminal:
```bash
qsteed-config
```

### Configure MySQL Service
#### 1. Install MySQL. 
You can download the appropriate [MySQL Community Server](https://dev.mysql.com/downloads/mysql/) from the MySQL official website.
For detailed installation instructions, see the official [documentation](https://dev.mysql.com/doc/refman/8.4/en/installing.html).

#### 2. Set MySQL user information
[//]: # (After installing QSteed, a folder named `QSteed` will be created in the root directory. )
[//]: # (Inside this folder, there is a configuration file called `config.ini`. )
Open the configuration file `config.ini` and enter your MySQL user information into the `mysql_config` property under section `[MySQL]`.
Please keep the following format:
```bash
mysql_config = {"host": "localhost",
                "user": "user_name",
                "password": "user_password",
                "database": "database_name"
               }
```

#### 3. Start MySQL service
Different platforms have different startup methods. For details, see [Getting Started with MySQL](https://dev.mysql.com/doc/mysql-getting-started/en/).

### Configure quantum chip information
#### 1. Add a chip
Open the configuration file `config.ini` in the QSteed folder. 
In section `[Chips]`, add your chip's basic information. 
For example, to add a chip named `"example"`, use the following format:
```bash
example = {"name": "example",
           "qubit_num": 10,
           "system_id": 0,
           "basis_gates": ["cx", "ry", "rz", "rx", "h", "id"]
           }
```

#### 2. Add the chip's size information
Add the chip's size information (embed the qubits into a two-dimensional grid) 
in the `chips_shape` property of section `[ChipsShape]`. Please keep the following format:
```bash
chips_shape = {
              "example": {"dimension": 1, "row": 1, "column": 10},
              }
```

#### 3. Add the mapping of the chip's name and ID.
Add the mapping in the `system_id_name` and `system_name_id` property of section `[Systems]`. Please keep the following format:
```bash
system_id_name = {0: "example",}
system_name_id = {"example": 0,}
```

For more examples of chip configuration, see the file [config.ini](qsteed/config/config.ini).

### Initialize the quantum computing resource virtualization database

> ⚠️<span style="color:#8B0000"> **Warning**</span>   
> If this is your first time installing QSteed, please make sure to perform the following database initialization steps after the installation is complete.

After the MySQL service starts and the `config.ini` file is configured, 
initialize the quantum computing resource virtualization database by running the following command:
```python
from qsteed.first_build_db import first_build_db
first_build_db()
```
or run the following command in the terminal:
```bash
qsteed-build-db
```

### Build or update database
We can build the quantum computing resource virtualization database
from the chip's json data file or the chip's information dictionary.
```python
from qsteed.apis.resourceDB_api import update_chip_api
import json
chip_file = 'chipexample.json'
with open(chip_file, 'r') as file:
    data_dict = json.load(file)
update_chip_api('example', data_dict)
```
For the data format of the chip, see file [chipexample.json](tests/chipexample.json) or 
[dongling.json](tests/dongling.json).


### Real machine deployment
QSteed has been successfully deployed to the [Quafu Quantum Cloud Computing Cluster](https://quafu.baqis.ac.cn/), 
supporting the compilation and optimization of user tasks into quantum circuits 
executable by quantum processors. 
You can submit tasks through the [Quafu Composer](https://quafu.baqis.ac.cn/#/composer) web interface,
[Pyquafu](https://scq-cloud.github.io/) client, 
or [QuarkStudio](https://www.yuque.com/wuming-6g8w2/ghzgfk/azika5xqlpxig3q3?singleDoc#) client.

If you would like to learn more about real machine deployment, please contact us.


## More Tests
See [tests](tests) for more examples.


## How to contribute
For information on how to contribute, please send an e-mail to members of developer of this project.

QSteed was developed by the quantum operating system team of the Beijing Academy of Quantum Information Sciences.

## License
QSteed is released under the Apache 2.0 license. See [LICENSE](LICENSE) for more details.
