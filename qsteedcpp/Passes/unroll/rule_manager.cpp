#include "rule_manager.h"

namespace qsteedcpp {

    void RuleManager::register_rule(const std::string& gate_name, const DecompositionRule& rule) {
        rules_[gate_name].push_back(rule);
        
        std::sort(rules_[gate_name].begin(), rules_[gate_name].end(),
                [](const auto& a, const auto& b) {
                    return a.priority < b.priority;
                });
    }
    
    void RuleManager::register_rule(const std::vector<std::string>& gate_names, const DecompositionRule& rule) {
        for (const auto& name : gate_names) {
            register_rule(name, rule);
        }
    }

    static bool is_subset(const std::vector<std::string>& subset, 
                         const std::set<std::string>& superset) {
        for (const auto& elem : subset) {
            if (superset.find(elem) == superset.end()) {
                return false;
            }
        }
        return true;
    }
    

    std::optional<RuleManager::DecompositionRule> RuleManager::select_rule(
        const std::string& gate_name,
        const std::set<std::string>& basis_gates) const {

        auto it = rules_.find(gate_name);
        if (it == rules_.end() || it->second.empty()) {
            return std::nullopt;
        }
        
        const auto& available_rules = it->second;

        // 第一阶段：寻找能直接分解到基础门的规则（优先级最高）
        auto direct_rule = std::find_if(available_rules.begin(), available_rules.end(),
            [&basis_gates](const DecompositionRule& rule) {
                return is_subset(rule.target_gates, basis_gates);
            });
        
        if (direct_rule != available_rules.end()) {
            return *direct_rule;
        }
        
        // 第二阶段：如果没有直接分解规则，返回优先级最高的规则
        return available_rules.front();
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

} // namespace qsteedcpp