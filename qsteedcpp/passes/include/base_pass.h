#pragma once
#include <memory>
#include <string>
#include "circuit/quantum_circuit.h"

namespace qsteedcpp {


    class BasePass {
    public:
        virtual ~BasePass() = default;
        
        virtual void run(QuantumCircuit& circuit) = 0;
        
        virtual std::string name() const = 0;
        
        virtual bool should_run(const QuantumCircuit& circuit) const { return true; }
    };

    using PassPtr = std::shared_ptr<BasePass>;

}; // namespace qsteedcpp