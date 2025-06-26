#pragma once
#include <string>
#include <map>
#include <autodiff/reverse/var.hpp>

namespace qsteedcpp {

using Parameter = autodiff::var;

inline Parameter get_parameter(const std::string& name, 
                              const std::map<std::string, double>& values, 
                              double default_value = 0.0) {
    auto it = values.find(name);
    return Parameter(it != values.end() ? it->second : default_value);
}

// 创建参数化函数
template<typename F>
auto make_parameterized_function(F&& func) {
    return [func = std::forward<F>(func)](const std::map<std::string, double>& params) {
        return func(params);
    };
}



// 参数表达式类
class ParameterExpression {
private:
    std::vector<std::variant<Parameter, double, std::string>> operands;
    std::vector<std::function<Parameter(Parameter, Parameter)>> operations;
    
public:
    void add_operand(const std::string& param_name);
    void add_operand(double constant);
    void add_operand(const Parameter& param);
    
    void add_operation(std::function<Parameter(Parameter, Parameter)> op);
    
    Parameter evaluate(const std::map<std::string, double>& context = {}) const;
};

} // namespace qsteedcpp 