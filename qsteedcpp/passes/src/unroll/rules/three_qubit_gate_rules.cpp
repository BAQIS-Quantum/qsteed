#include "unroll/rules/three_qubit_gate_rules.h"
#include "unroll/rule_manager.h"
#include "unroll/decomposition_rules.h"
#include "circuit/circuit_instruction.h"
#include "QuantumCircuit/include/gates/standard_gates.h"

namespace qsteedcpp {

namespace { // Anonymous namespace for local helpers and rule implementations

    void apply_condition(const CircuitInstruction& source, std::vector<CircuitInstruction>& targets) {
        if (source.condition.has_value()) {
            for (auto& target : targets) {
                target.condition = source.condition;
            }
        }
    }

    /**
     * 将Toffoli门分解为CNOT门和单量子比特门的序列
     * 使用标准的Toffoli门分解算法，将三量子比特门转换为双量子比特门和单量子比特门的组合
     * @param inst 输入的Toffoli门指令，包含三个量子比特索引
     * @return 分解后的指令序列，包含H门、CNOT门、T门和T†门
     */
    std::vector<CircuitInstruction> toffoli_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        int ctrl1 = qubits[0];
        int ctrl2 = qubits[1];
        int targ = qubits[2];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{ctrl2});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{ctrl1});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{ctrl2});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        
        apply_condition(inst, result);
        return result;
    }

    /**
     * 将Toffoli门分解为线性拓扑结构的CNOT门和单量子比特门序列
     * 使用线性分解算法，适用于量子比特排列为线性结构的硬件
     * 相比标准分解，这种分解在特定硬件拓扑下可能具有更好的性能
     * @param inst 输入的Toffoli门指令，包含三个量子比特索引
     * @return 分解后的指令序列，包含H门、CNOT门、T门和T†门
     */
    std::vector<CircuitInstruction> toffoli_to_cnot_linear(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        int ctrl1 = qubits[0];
        int ctrl2 = qubits[1];
        int targ = qubits[2];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{ctrl1});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{ctrl2});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{ctrl2});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{targ});
        
        apply_condition(inst, result);
        return result;
    }

    /**
     * 将Fredkin门（受控交换门）分解为Toffoli门和CNOT门的组合
     * Fredkin门是一个三量子比特门，当控制量子比特为|1⟩时，交换两个目标量子比特
     * 通过使用Toffoli门和CNOT门可以实现相同的功能
     * @param inst 输入的Fredkin门指令，包含三个量子比特索引
     * @return 分解后的指令序列，包含CNOT门和Toffoli门
     */
    std::vector<CircuitInstruction> fredkin_to_toffoli(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // Fredkin门分解：使用CNOT门和Toffoli门的组合实现受控交换
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[2], qubits[1]});
        result.emplace_back(std::make_unique<ToffoliGate>(), qubits);
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[2], qubits[1]});
        
        apply_condition(inst, result);
        return result;
    }

} // anonymous namespace

void initialize_three_qubit_gate_rules(RuleManager& manager) {
    manager.register_rule(std::vector<std::string>{"ccx", "toffoli"}, {
        .name = "toffoli_to_cnot_linear",
        .target_gates = {"cnot", "h", "t", "tdg"},
        .decomposer = toffoli_to_cnot_linear,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule(std::vector<std::string>{"ccx", "toffoli"}, {
        .name = "toffoli_to_cnot",
        .target_gates = {"cnot", "h", "t", "tdg"},
        .decomposer = toffoli_to_cnot,
        .global_phase = 0.0,
        .priority = 2
    });
    manager.register_rule(std::vector<std::string>{"fredkin", "cswap"}, {
        .name = "fredkin_to_toffoli",
        .target_gates = {"cx", "ccx"},
        .decomposer = fredkin_to_toffoli,
        .global_phase = 0.0,
        .priority = 1
    });
}

} // namespace qsteedcpp