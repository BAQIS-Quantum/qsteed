#pragma once
#include <functional>
#include <string>
#include <vector>

namespace qsteedcpp {

class RuleManager;

// Initializes all standard decomposition rules.
void initialize_standard_rules(RuleManager& manager);

} // namespace qsteedcpp
