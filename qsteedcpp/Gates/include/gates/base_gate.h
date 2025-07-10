#pragma once

#include "../matrix.h"
#include "parameter.h"
#include <string>
#include <vector>
#include <map>

namespace qsteedcpp {

// 门的基类
class Gate {
private:
    std::string name_;
    int qubit_count_;
    std::vector<Parameter> parameters_;
    
public:
    Gate(const std::string& name, int qubit_count) 
        : name_(name), qubit_count_(qubit_count) {}
    
    Gate(const std::string& name, int qubit_count, const std::vector<Parameter>& params)
        : name_(name), qubit_count_(qubit_count), parameters_(params) {}
    
    virtual ~Gate() = default;
    
    std::string get_name() const { return name_; }
    int get_qubit_count() const { return qubit_count_; }
    
    bool has_parameters() const { return !parameters_.empty(); }
    size_t parameter_count() const { return parameters_.size(); }
    
    const Parameter& get_parameter(size_t index) const { return parameters_[index]; }
    void set_parameter(size_t index, const Parameter& param) { 
        parameters_[index] = param; 
    }
    
    std::vector<double> get_parameter_values(const std::map<std::string, double>& param_map = {}) const {
        std::vector<double> values;
        for (const auto& param : parameters_) {
            values.push_back(param.value(param_map));
        }
        return values;
    }
    
        
    Matrix apply(const Matrix& state, const std::map<std::string, double>& param_map = {}) const {
        return get_matrix(param_map) * state;
    }

    // 获取矩阵（纯虚函数，子类必须实现）
    virtual Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const = 0;

};

} // namespace qsteedcpp