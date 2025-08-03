#include <algorithm>
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

    // 辅助函数：检查一个集合是否是另一个集合的子集
    static bool is_subset(const std::vector<std::string>& subset, 
                         const std::set<std::string>& superset) {
        for (const auto& elem : subset) {
            if (superset.find(elem) == superset.end()) {
                return false;
            }
        }
        return true;
    }
    
    std::optional<RuleManager::DecompositionRule> 
    RuleManager::select_rule(const std::string& gate_name,
                            const std::set<std::string>& basis_gates) const {
        auto it = rules_.find(gate_name);
        if (it == rules_.end() || it->second.empty()) {
            return std::nullopt;
        }
        
        const auto& all_rules = it->second;
        
        // 第一阶段：寻找能直接分解到 basis_gates 的规则
        std::vector<DecompositionRule> direct_rules;
        for (const auto& rule : all_rules) {
            if (is_subset(rule.target_gates, basis_gates)) {
                direct_rules.push_back(rule);
            }
        }
        
        if (!direct_rules.empty()) {
            return direct_rules.front();
        }
        
        return all_rules.front();
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