#pragma once

#include <string>
#include <vector>
#include <map>
#include "circuit/parameter.h"
#include "matrix.h"

namespace qsteedcpp {


class Gate {
private:
    int qubit_count_;
    std::vector<Parameter> parameters_;
    
public:
    Gate(int qubit_count) 
        : qubit_count_(qubit_count) {}
    
    Gate(int qubit_count, const std::vector<Parameter>& params)
        : qubit_count_(qubit_count), parameters_(params) {}
    
    virtual ~Gate() = default;
    
    // 纯虚函数，每个派生类必须提供自己的静态名称
    virtual const char* name() const = 0;
    
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
    
    void update_parameters(const std::map<std::string, double>& param_values) {
        for (auto& param : parameters_) {
            // 对于每个参数，如果它是一个简单的变量且在param_values中，则更新它
            auto param_vars = param.get_variables();
            if (param_vars.size() == 1 && param_values.find(param_vars[0]) != param_values.end()) {
                param.set_value(param_values.at(param_vars[0]));
            }
        }
    }
    
        
    // Matrix
    Matrix apply(const Matrix& state, const std::map<std::string, double>& param_map = {}) const {
        return get_matrix(param_map) * state;
    }

    virtual Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const = 0;
    
    virtual std::unique_ptr<Gate> clone() const = 0;
};


template<typename Derived>
class ClonableGate : public Gate {
public:
    using Gate::Gate;
    
    std::unique_ptr<Gate> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

} // namespace qsteedcpp