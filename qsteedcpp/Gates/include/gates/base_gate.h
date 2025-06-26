#pragma once

#include "../matrix.h"
#include "parameter.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace qsteedcpp {

class QuantumGate {
protected:
    std::vector<std::unique_ptr<Parameter>> parameters_;

public:
    virtual ~QuantumGate() = default;
    
    virtual std::string get_name() const = 0;
    
    virtual int get_qubit_count() const = 0;
    
    // 获取矩阵（使用当前参数值）
    virtual Matrix get_matrix() const = 0;
    
    // 获取参数化矩阵（提供参数值）
    virtual Matrix get_matrix(const std::map<std::string, double>& parameters) const = 0;

    virtual Matrix apply(const Matrix& state) const = 0;
     
};

} // namespace qsteedcpp