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
        std::vector<std::string> target_gates;
        std::function<std::vector<CircuitInstruction>(const CircuitInstruction&)> decomposer;
        double global_phase = 0.0;
        int priority = 0;           
    };

private:
    std::unordered_map<std::string, std::vector<DecompositionRule>> rules_;

public:
    void register_rule(const std::string& gate_name, const DecompositionRule& rule);
    
    // 返回最高优先级的规则（不再检查 basis_gates）
    std::optional<DecompositionRule> select_rule(const std::string& gate_name) const;
    
    // 获取某个门的所有规则
    std::vector<DecompositionRule> get_rules(const std::string& gate_name) const;
    
    // 检查是否有某个门的规则
    bool has_rule(const std::string& gate_name) const;

    // 清空所有规则
    void clear();
};

} // namespace qsteedcpp