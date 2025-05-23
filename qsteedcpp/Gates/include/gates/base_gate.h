#pragma once

#include "../matrix.h"
#include <string>
#include <vector>

namespace Gate {

class QuantumGate {
public:
    virtual ~QuantumGate() = default;
    
    virtual std::string get_name() const = 0;
    
    virtual int get_qubit_count() const = 0;
    
    virtual Matrix get_matrix() const = 0;

    virtual Matrix apply(const Matrix& state) const = 0;
};

} // namespace Gate