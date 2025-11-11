#include "decomposition_rules.h"
#include "rules/swap_rules.h"
#include "rules/single_qubit_rules.h"
#include "rules/two_qubit_rules.h"
#include "rules/controlled_rotation_rules.h"
#include "rules/three_qubit_gate_rules.h"

namespace qsteedcpp {

    void initialize_standard_rules(RuleManager& manager) {
        initialize_swap_rules(manager);
        initialize_single_qubit_rules(manager);
        initialize_two_qubit_rules(manager);
        initialize_controlled_rotation_rules(manager);
        initialize_three_qubit_gate_rules(manager);
    }

} // namespace qsteedcpp
