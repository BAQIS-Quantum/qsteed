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

from quafu.elements.parameters import Parameter, ParameterExpression

from qsteed.dag.circuit_dag_convert import dag_to_circuit
from qsteed.dag.dagcircuit import DAGCircuit
from qsteed.passes.basepass import BasePass
from functools import lru_cache


class ParaSubstitution(BasePass):
    """
    Substitute the parameters in the circuit with the original parameters
    """

    def __init__(self):
        super().__init__()

        self.para_list = []
        self.para_dict = dict()

    def set_model(self, model):
        """
        Set the model, including information such as backend and parameters.
        here we only need the parameters in the model
        """
        self.para_list = model.get_datadict()['variables']

    def run(self, circuit):
        """
        create a dict to store the original variable parameters and their names as keys, and the original parameters as values
        update  Parameter variables in the circuit gobally, and
        update Parameter variables in the circuit gates,
        return the updated circuit

        Args:
            circuit: the input circuit

        Returns:
            QuantumCircuit: the updated circuit
        """
        if isinstance(circuit, DAGCircuit):
            circuit = dag_to_circuit(circuit, circuit.circuit_qubits)

        para_dict = dict()
        for para in self.para_list:
            para_dict[f'{para.name}'] = para

        self.para_dict = para_dict

        # update the global variables in the circuit
        self.update_global_variables(circuit)

        # update the parameters in the circuit gates
        self.circuit_update_gates_para(circuit)

        circuit.get_parameter_grads()  # this is optional,you can use the function after  all the compilation passes

        # return the updated quantum circuit.if the input is DAGCircuit, return the new quantum circuit
        return circuit

    def update_global_variables(self, circuit):
        """
        update the global variables in the circuit
        """
        circuit._variables = list(self.para_dict.values())

    def circuit_update_gates_para(self, circuit):
        """
        Update all gate parameters in the circuit.
        """

        for gate in circuit.gates:
            gate = self.update_gate_para(gate)

    def update_gate_para(self, gate):
        """
        Update the parameters in the gate.
        """
        for i, para in enumerate(gate.paras):
            if isinstance(para, (Parameter, ParameterExpression)):
                gate.paras[i] = self.update_para_variables(para)
        return gate

    @lru_cache(maxsize=128)
    def update_para_variables(self, para):
        """
        Update the parameters in the Parameter or ParameterExpression variable.
        """
        if isinstance(para, Parameter):
            para = self.para_dict[f'{para.name}']
            return para

        elif isinstance(para, ParameterExpression):
            para.pivot = self.update_para_variables(para.pivot)
            for i, operand in enumerate(para.operands):
                if isinstance(operand, (Parameter, ParameterExpression)):
                    para.operands[i] = self.update_para_variables(operand)

            return para

        else:
            return para


class ParaSubstitutionCached(BasePass):
    """
    Substitute the parameters in the circuit with the original parameters
    """

    def __init__(self):
        super().__init__()
        self.para_list = []
        self.para_dict = dict()
        self._para_dict_cache = None
        self._gate_para_cache = {}

    def set_model(self, model):
        """
        Set the model, including information such as backend and parameters.
        Here we only need the parameters in the model
        """
        self.para_list = model.get_datadict()['variables']
        self._para_dict_cache = None  # Invalidate the cache

    def run(self, circuit):
        """
        Create a dict to store the original variable parameters and their names as keys, and the original parameters as values
        Update Parameter variables in the circuit globally, and
        Update Parameter variables in the circuit gates,
        Return the updated circuit

        Args:
            circuit: the input circuit

        Returns:
            QuantumCircuit: the updated circuit
        """
        if isinstance(circuit, DAGCircuit):
            circuit = dag_to_circuit(circuit, circuit.circuit_qubits)

        if self._para_dict_cache is None:
            para_dict = {f'{para.name}': para for para in self.para_list}
            self.para_dict = para_dict
            self._para_dict_cache = para_dict
        else:
            self.para_dict = self._para_dict_cache

        # Update the global variables in the circuit
        self.update_global_variables(circuit)

        # Update the parameters in the circuit gates
        self.circuit_update_gates_para(circuit)

        circuit.get_parameter_grads()  # This is optional; you can use the function after all the compilation passes

        # Return the updated quantum circuit. If the input is DAGCircuit, return the new quantum circuit
        return circuit

    def update_global_variables(self, circuit):
        """
        Update the global variables in the circuit
        """
        circuit._variables = list(self.para_dict.values())

    def circuit_update_gates_para(self, circuit):
        """
        Update all gate parameters in the circuit.
        """
        for gate in circuit.gates:
            gate = self.update_gate_para(gate)

    def update_gate_para(self, gate):
        """
        Update the parameters in the gate.
        """
        gate_id = id(gate)
        if gate_id not in self._gate_para_cache:
            for i, para in enumerate(gate.paras):
                if isinstance(para, (Parameter, ParameterExpression)):
                    gate.paras[i] = self.update_para_variables(para)
            self._gate_para_cache[gate_id] = gate
        return self._gate_para_cache[gate_id]

    def update_para_variables(self, para):
        """
        Update the parameters in the Parameter or ParameterExpression variable.
        """
        if isinstance(para, Parameter):
            para = self.para_dict[f'{para.name}']
            return para

        elif isinstance(para, ParameterExpression):
            para.pivot = self.update_para_variables(para.pivot)
            for i, operand in enumerate(para.operands):
                if isinstance(operand, (Parameter, ParameterExpression)):
                    para.operands[i] = self.update_para_variables(operand)

            return para

        else:
            return para
