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
    
    std::optional<DecompositionRule> select_rule(
        const std::string& gate_name,
        const std::set<std::string>& basis_gates) const;
    
    std::vector<DecompositionRule> get_rules(const std::string& gate_name) const;
    
    bool has_rule(const std::string& gate_name) const;

    void clear();

private:
    bool is_compatible(const DecompositionRule& rule,
                      const std::set<std::string>& basis_gates) const;
};

} // namespace qsteedcpp