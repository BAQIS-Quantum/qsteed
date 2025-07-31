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

    std::optional<RuleManager::DecompositionRule> 
    RuleManager::select_rule(const std::string& gate_name) const {
        auto it = rules_.find(gate_name);
        if (it == rules_.end() || it->second.empty()) {
            return std::nullopt;
        }
        
        // 返回优先级最高的规则（已经按优先级排序）
        return it->second.front();
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