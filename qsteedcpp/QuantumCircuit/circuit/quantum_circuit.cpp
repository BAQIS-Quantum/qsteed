#include <iostream>
#include <sstream>
#include "quantum_circuit.h"
#include "QuantumCircuit/gates/standard_gates.h"
#include "circuit_drawer.h"
#include "Qarser/parser.h"
#include "Qarser/SA/analyzer.hpp"
#include "Qarser/AST/qasm_to_circuit.hpp"

namespace qsteedcpp {

void QuantumCircuit::validate_qubit_index(int qubit) const {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::out_of_range("Qubit index " + std::to_string(qubit) + " out of range [0, " + std::to_string(num_qubits_) + ")");
    }
}

void QuantumCircuit::validate_clbit_index(int clbit) const {
    if (clbit < 0 || clbit >= num_clbits_) {
        throw std::out_of_range("Classical bit index " + std::to_string(clbit) + " out of range [0, " + std::to_string(num_clbits_) + ")");
    }
}

void QuantumCircuit::validate_gate_qubits(const std::vector<int>& qubits, const std::string& gate_name) {
    for (int qubit : qubits) {
        validate_qubit_index(qubit);
    }
    
    if (qubits.size() > 1) {
        // Using a simple loop for checking duplicates for small number of qubits
        for (size_t i = 0; i < qubits.size(); ++i) {
            for (size_t j = i + 1; j < qubits.size(); ++j) {
                if (qubits[i] == qubits[j]) {
                    throw std::invalid_argument(gate_name + " cannot operate on duplicate qubits");
                }
            }
        }
    }
}


// Gates
void QuantumCircuit::h(int qubit) {
    add_gate(std::make_unique<HGate>(), {qubit});
}

void QuantumCircuit::x(int qubit) {
    add_gate(std::make_unique<XGate>(), {qubit});
}

void QuantumCircuit::y(int qubit) {
    add_gate(std::make_unique<YGate>(), {qubit});
}

void QuantumCircuit::z(int qubit) {
    add_gate(std::make_unique<ZGate>(), {qubit});
}

void QuantumCircuit::rx(const Expr& theta, int qubit) {
    add_gate(std::make_unique<RXGate>(theta), {qubit});
}

void QuantumCircuit::ry(const Expr& phi, int qubit) {
    add_gate(std::make_unique<RYGate>(phi), {qubit});
}

void QuantumCircuit::rz(const Expr& lambda, int qubit) {
    add_gate(std::make_unique<RZGate>(lambda), {qubit});
}

void QuantumCircuit::cnot(int control, int target) {
    add_gate(std::make_unique<CNOTGate>(), {control, target});
}

void QuantumCircuit::rxx(const Expr& theta, int qubit1, int qubit2) {
    add_gate(std::make_unique<RXXGate>(theta), {qubit1, qubit2});
}

void QuantumCircuit::ryy(const Expr& theta, int qubit1, int qubit2) {
    add_gate(std::make_unique<RYYGate>(theta), {qubit1, qubit2});
}

void QuantumCircuit::rzz(const Expr& theta, int qubit1, int qubit2) {
    add_gate(std::make_unique<RZZGate>(theta), {qubit1, qubit2});
}

void QuantumCircuit::s(int qubit) {
    add_gate(std::make_unique<SGate>(), {qubit});
}

void QuantumCircuit::sdg(int qubit) {
    add_gate(std::make_unique<SdgGate>(), {qubit});
}

void QuantumCircuit::t(int qubit) {
    add_gate(std::make_unique<TGate>(), {qubit});
}

void QuantumCircuit::tdg(int qubit) {
    add_gate(std::make_unique<TdgGate>(), {qubit});
}

void QuantumCircuit::cz(int control, int target) {
    add_gate(std::make_unique<CZGate>(), {control, target});
}

void QuantumCircuit::swap(int qubit1, int qubit2) {
    add_gate(std::make_unique<SwapGate>(), {qubit1, qubit2});
}

void QuantumCircuit::iswap(int qubit1, int qubit2) {
    add_gate(std::make_unique<ISwapGate>(), {qubit1, qubit2});
}

void QuantumCircuit::ccx(int control1, int control2, int target) {
    add_gate(std::make_unique<ToffoliGate>(), {control1, control2, target});
}

void QuantumCircuit::toffoli(int control1, int control2, int target) {
    ccx(control1, control2, target);
}

void QuantumCircuit::p(const Expr& lambda, int qubit) {
    add_gate(std::make_unique<PhaseGate>(lambda), {qubit});
}

void QuantumCircuit::u3(const Expr& theta, const Expr& phi, const Expr& lambda, int qubit) {
    add_gate(std::make_unique<U3Gate>(theta, phi, lambda), {qubit});
}

// Non-gate operations
void QuantumCircuit::measure(int qubit, int clbit) {
    validate_qubit_index(qubit);
    validate_clbit_index(clbit);
    instructions_.emplace_back(Measurement(qubit, clbit));
}

void QuantumCircuit::measure(const std::vector<int>& qubits, const std::vector<int>& clbits) {
    if (qubits.size() != clbits.size()) {
        throw std::invalid_argument("Number of qubits and clbits must match in measurement");
    }

    for (int qubit : qubits) {
        validate_qubit_index(qubit);
    }
    for (int clbit : clbits) {
        validate_clbit_index(clbit);
    }

    instructions_.emplace_back(Measurement(qubits, clbits));
}

void QuantumCircuit::measure(const std::map<int, int>& qubit_clbit_map) {
    std::vector<int> qubits;
    std::vector<int> clbits;

    for (const auto& [qubit, clbit] : qubit_clbit_map) {
        qubits.push_back(qubit);
        clbits.push_back(clbit);
    }

    measure(qubits, clbits);
}

void QuantumCircuit::measure_all() {
    if (num_qubits_ > num_clbits_) {
        throw std::invalid_argument("Not enough classical bits to measure all qubits");
    }
    
    std::vector<int> qubits(num_qubits_);
    std::vector<int> clbits(num_qubits_);
    
    for (int i = 0; i < num_qubits_; ++i) {
        qubits[i] = i;
        clbits[i] = i;
    }
    
    measure(qubits, clbits);
}

std::vector<CircuitInstruction> QuantumCircuit::get_gates() const {
    std::vector<CircuitInstruction> gates_only;
    gates_only.reserve(instructions_.size()); // Pre-allocate for efficiency

    for (const auto& inst : instructions_) {
        if (inst.is_gate()) {
            gates_only.push_back(inst);
        }
    }

    return gates_only;
}

void QuantumCircuit::barrier(const std::vector<int>& qubits) {
    std::vector<int> barrier_qubits = qubits;
    if (barrier_qubits.empty()) {
        for (int i = 0; i < num_qubits_; ++i) {
            barrier_qubits.push_back(i);
        }
    } else {
        for (int q : barrier_qubits) {
            validate_qubit_index(q);
        }
    }
    instructions_.emplace_back(Barrier(barrier_qubits));
}

void QuantumCircuit::reset(int qubit) {
    validate_qubit_index(qubit);
    instructions_.emplace_back(Reset(qubit));
}

void QuantumCircuit::delay(int qubit, int duration, const std::string& unit) {
    validate_qubit_index(qubit);
    instructions_.emplace_back(Delay(qubit, duration, unit));
}

void QuantumCircuit::xy(int qubit_start, int qubit_end, int duration, const std::string& unit) {
    validate_qubit_index(qubit_start);
    validate_qubit_index(qubit_end);
    instructions_.emplace_back(XYResonance(qubit_start, qubit_end, duration, unit));
}

void QuantumCircuit::append(const CircuitInstruction& instruction) {
    // Validation
    for (int q : instruction.qubits) {
        validate_qubit_index(q);
    }
    for (int c : instruction.clbits) {
        validate_clbit_index(c);
    }
    if (instruction.is_gate()) {
        validate_gate_qubits(instruction.qubits, instruction.name());
    }

    instructions_.push_back(instruction);
}

void QuantumCircuit::append_c_if(std::unique_ptr<Gate> gate, const std::vector<int>& qubits, int clbit, int value) {
    validate_clbit_index(clbit);
    CircuitInstruction inst(std::move(gate), qubits);
    inst.c_if(clbit, value);
    instructions_.push_back(std::move(inst));
}

void QuantumCircuit::add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits) {
    if (static_cast<int>(qubits.size()) != gate->get_qubit_count()) {
        throw std::invalid_argument("Number of qubits doesn't match gate requirement");
    }
    
    validate_gate_qubits(qubits, gate->name());
    instructions_.emplace_back(std::move(gate), qubits);
}

void QuantumCircuit::print() const {
    CircuitDrawer drawer(instructions_, num_qubits_, num_clbits_);
    std::cout << drawer.draw() << std::endl;
}

std::set<std::string> QuantumCircuit::get_all_parameter_uuids() const {
    std::set<std::string> all_uuids;
    for (const auto& inst : instructions_) {
        if (inst.is_gate()) {
            const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
            auto gate_uuids = gate->get_parameter_uuids();
            all_uuids.insert(gate_uuids.begin(), gate_uuids.end());
        }
    }
    return all_uuids;
}


// Helper functions for get_variables
namespace {
    struct ParameterUUIDComp {
        bool operator()(const Parameter& a, const Parameter& b) const {
            return a.get_uuid() < b.get_uuid();
        }
    };

    void collect_parameters_recursive(const Expression* expr_node, std::set<Parameter, ParameterUUIDComp>& params) {
        if (!expr_node) return;

        switch (expr_node->get_type()) {
            case Expression::Type::PARAMETER: {
                const auto* param_ptr = dynamic_cast<const Parameter*>(expr_node);
                if (param_ptr) {
                    params.insert(*param_ptr);
                }
                break;
            }
            case Expression::Type::BINARY_OP: {
                const auto* bin_op = dynamic_cast<const qsteedcpp::BinaryOp*>(expr_node);
                if (bin_op) {
                    collect_parameters_recursive(bin_op->get_left(), params);
                    collect_parameters_recursive(bin_op->get_right(), params);
                }
                break;
            }
            case Expression::Type::UNARY_OP: {
                const auto* un_op = dynamic_cast<const qsteedcpp::UnaryOp*>(expr_node);
                if (un_op) {
                    collect_parameters_recursive(un_op->get_operand(), params);
                }
                break;
            }
            case Expression::Type::CONSTANT:
                // Do nothing
                break;
        }
    }
}

std::vector<Parameter> QuantumCircuit::get_variables() const {
    std::set<Parameter, ParameterUUIDComp> unique_params;

    for (const auto& inst : instructions_) {
        if (inst.is_gate()) {
            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(inst.operation);
            if (gate_ptr->has_parameters()) {
                for (const auto& expr : gate_ptr->get_parameter_expressions()) {
                    collect_parameters_recursive(expr.get(), unique_params);
                }
            }
        }
    }

    std::vector<Parameter> result;
    result.reserve(unique_params.size());
    for(const auto& p : unique_params) {
        result.push_back(p);
    }
    return result;
}

QuantumCircuit QuantumCircuit::from_openqasm(const std::string& qasm_str) {
    qarser::Parser parser(qasm_str);
    std::unique_ptr<qarser::Program> program = parser.parse();

    qarser::SemanticAnalyzer analyzer;
    analyzer.analyze(*program);

    qarser::QasmToCircuit converter;
    std::unique_ptr<QuantumCircuit> circuit = converter.convert(*program);

    return std::move(*circuit);
}


std::string QuantumCircuit::to_openqasm(bool with_para) const {
    std::stringstream ss;
    ss << "OPENQASM 2.0;" << std::endl;
    ss << "include \"qelib1.inc\";" << std::endl;
    ss << "qreg q[" << num_qubits_ << "];" << std::endl;
    if (num_clbits_ > 0) {
        ss << "creg c[" << num_clbits_ << "];" << std::endl;
    }

    for (const auto& inst : instructions_) {
        std::string qasm_line = inst.to_qasm(with_para);
        if (!qasm_line.empty()) {
            ss << qasm_line << std::endl;
        }
    }

    return ss.str();
}

} // namespace qsteedcpp
