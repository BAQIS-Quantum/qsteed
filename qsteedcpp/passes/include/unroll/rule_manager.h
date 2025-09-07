#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <optional>
#include <set>
#include "circuit/circuit_instruction.h"

namespace qsteedcpp {

class RuleManager {
public:
    struct DecompositionRule {
        std::string name;
        // target_gates: 此规则分解的直接产物
        // 用于上下文感知的规则选择 - 优先选择能一步分解到基础门的规则
        std::vector<std::string> target_gates;
        std::function<std::vector<CircuitInstruction>(const CircuitInstruction&)> decomposer;
        double global_phase = 0.0;
        int priority = 0;
    };

private:
    std::unordered_map<std::string, std::vector<DecompositionRule>> rules_;

public:
    void register_rule(const std::string& gate_name, const DecompositionRule& rule);
    
    void register_rule(const std::vector<std::string>& gate_names, const DecompositionRule& rule);
    

    // 第一阶段：寻找能一步分解到 basis_gates 的规则
    // 第二阶段：如果没有找到，返回优先级最高的规则（允许多步分解）
    std::optional<DecompositionRule> select_rule(
        const std::string& gate_name,
        const std::set<std::string>& basis_gates) const;
    
    std::vector<DecompositionRule> get_rules(const std::string& gate_name) const;
    
    bool has_rule(const std::string& gate_name) const;

    // clear all rules
    void clear();
};

} // namespace qsteedcpp