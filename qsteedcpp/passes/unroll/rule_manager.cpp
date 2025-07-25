#include "rule_manager.h"
#include <algorithm>

namespace qsteedcpp {

void RuleManager::register_rule(const std::string& gate_name, const DecompositionRule& rule) {
    rules_[gate_name].push_back(rule);
    
    std::sort(rules_[gate_name].begin(), rules_[gate_name].end(),
             [](const auto& a, const auto& b) {
                 return a.priority < b.priority;
             });
}

std::optional<RuleManager::DecompositionRule> 
RuleManager::select_rule(const std::string& gate_name,
                        const std::set<std::string>& basis_gates) const {
    
    auto it = rules_.find(gate_name);
    if (it == rules_.end()) {
        return std::nullopt;
    }
    
    for (const auto& rule : it->second) {
        if (is_compatible(rule, basis_gates)) {
            return rule;
        }
    }
    
    return std::nullopt;
}

bool RuleManager::has_rule(const std::string& gate_name) const {
    return rules_.count(gate_name) > 0;
}

std::vector<RuleManager::DecompositionRule> 
RuleManager::get_rules(const std::string& gate_name) const {
    auto it = rules_.find(gate_name);
    if (it != rules_.end()) {
        return it->second;
    }
    return {};
}

void RuleManager::clear() {
    rules_.clear();
}

bool RuleManager::is_compatible(const DecompositionRule& rule,
                               const std::set<std::string>& basis_gates) const {
    return std::all_of(rule.target_gates.begin(), 
                      rule.target_gates.end(),
                      [&basis_gates](const std::string& gate) {
                          return basis_gates.count(gate) > 0;
                      });
}

} // namespace qsteedcpp