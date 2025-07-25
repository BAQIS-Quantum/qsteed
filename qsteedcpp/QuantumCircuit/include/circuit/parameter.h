#pragma once
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <autodiff/reverse/var.hpp>
#include "circuit/uuid_generator.h"

namespace qsteedcpp {

using namespace autodiff;


class ExpressionNode {
public:
    virtual ~ExpressionNode() = default;
    virtual double evaluate(const std::map<std::string, double>& variable_values) const = 0;
    virtual std::vector<std::string> get_variables() const = 0;
    virtual std::unique_ptr<ExpressionNode> clone() const = 0;
    virtual std::string to_string() const = 0;
    virtual var evaluate_autodiff(const std::map<std::string, double>& variable_values,
                                 const std::string& target_variable,
                                 var target_value) const = 0;
    
    // 获取参数UUID到变量名的映射
    virtual std::map<std::string, std::string> get_parameter_map() const {
        return {};
    }
};

class ConstantNode : public ExpressionNode {
private:
    double value_;
public:
    explicit ConstantNode(double value) : value_(value) {}
    
    double evaluate(const std::map<std::string, double>& variable_values) const override {
        return value_;
    }
    
    std::vector<std::string> get_variables() const override {
        return {};
    }
    
    std::unique_ptr<ExpressionNode> clone() const override {
        return std::make_unique<ConstantNode>(value_);
    }
    
    std::string to_string() const override {
        return std::to_string(value_);
    }

    var evaluate_autodiff(const std::map<std::string, double>& variable_values,
                         const std::string& target_variable,
                         var target_value) const override {
        return value_;
    }
};


class VariableNode : public ExpressionNode {
private:
    std::string name_;
    std::string uuid_;  // 参数的唯一标识（如果是参数变量）
    
public:
    // 普通变量构造函数（没有关联的 Parameter）
    explicit VariableNode(const std::string& name) : name_(name) {}
    
    // 参数变量构造函数（有关联的 Parameter）
    VariableNode(const std::string& name, const std::string& uuid) 
        : name_(name), uuid_(uuid) {}
    
    double evaluate(const std::map<std::string, double>& variable_values) const override {
        auto it = variable_values.find(name_);
        if (it != variable_values.end()) {
            return it->second;
        }
        return 0.0; // 默认值
    }
    
    std::vector<std::string> get_variables() const override {
        return {name_};
    }
    
    std::unique_ptr<ExpressionNode> clone() const override {
        if (uuid_.empty()) {
            return std::make_unique<VariableNode>(name_);
        } else {
            return std::make_unique<VariableNode>(name_, uuid_);
        }
    }
    
    std::map<std::string, std::string> get_parameter_map() const override {
        std::map<std::string, std::string> map;
        if (!uuid_.empty()) {
            map[uuid_] = name_;  // UUID -> 变量名的映射
        }
        return map;
    }
    
    std::string to_string() const override {
        return name_;
    }

    var evaluate_autodiff(const std::map<std::string, double>& variable_values,
                         const std::string& target_variable,
                         var target_value) const override {
        if (name_ == target_variable) {
            return target_value;
        } else {
            auto it = variable_values.find(name_);
            if (it != variable_values.end()) {
                return it->second;
            }
            return 0.0;
        }
    }
};

class BinaryOpNode : public ExpressionNode {
private:
    std::unique_ptr<ExpressionNode> left_;
    std::unique_ptr<ExpressionNode> right_;
    std::function<double(double, double)> op_;
    std::string op_symbol_;
    
public:
    BinaryOpNode(std::unique_ptr<ExpressionNode> left, 
                 std::unique_ptr<ExpressionNode> right,
                 std::function<double(double, double)> op,
                 const std::string& op_symbol)
        : left_(std::move(left)), right_(std::move(right)), 
          op_(op), op_symbol_(op_symbol) {}
    
    double evaluate(const std::map<std::string, double>& variable_values) const override {
        return op_(left_->evaluate(variable_values), right_->evaluate(variable_values));
    }
    
    std::vector<std::string> get_variables() const override {
        auto left_vars = left_->get_variables();
        auto right_vars = right_->get_variables();
        left_vars.insert(left_vars.end(), right_vars.begin(), right_vars.end());
        return left_vars;
    }
    
    std::unique_ptr<ExpressionNode> clone() const override {
        return std::make_unique<BinaryOpNode>(left_->clone(), right_->clone(), op_, op_symbol_);
    }
    
    std::string to_string() const override {
        return "(" + left_->to_string() + " " + op_symbol_ + " " + right_->to_string() + ")";
    }
    
    std::map<std::string, std::string> get_parameter_map() const override {
        auto left_map = left_->get_parameter_map();
        auto right_map = right_->get_parameter_map();
        // 合并两个映射
        left_map.insert(right_map.begin(), right_map.end());
        return left_map;
    }

    var evaluate_autodiff(const std::map<std::string, double>& variable_values,
                         const std::string& target_variable,
                         var target_value) const override {
        var left_val = left_->evaluate_autodiff(variable_values, target_variable, target_value);
        var right_val = right_->evaluate_autodiff(variable_values, target_variable, target_value);
        
        if (op_symbol_ == "+") {
            return left_val + right_val;
        } else if (op_symbol_ == "-") {
            return left_val - right_val;
        } else if (op_symbol_ == "*") {
            return left_val * right_val;
        } else if (op_symbol_ == "/") {
            return left_val / right_val;
        } else {
            return left_val + right_val;
        }
    }
};


class UnaryOpNode : public ExpressionNode {
private:
    std::unique_ptr<ExpressionNode> operand_;
    std::function<double(double)> op_;
    std::string op_symbol_;
    
public:
    UnaryOpNode(std::unique_ptr<ExpressionNode> operand,
                std::function<double(double)> op,
                const std::string& op_symbol)
        : operand_(std::move(operand)), op_(op), op_symbol_(op_symbol) {}
    
    double evaluate(const std::map<std::string, double>& variable_values) const override {
        return op_(operand_->evaluate(variable_values));
    }
    
    std::vector<std::string> get_variables() const override {
        return operand_->get_variables();
    }
    
    std::unique_ptr<ExpressionNode> clone() const override {
        return std::make_unique<UnaryOpNode>(operand_->clone(), op_, op_symbol_);
    }
    
    std::string to_string() const override {
        return op_symbol_ + "(" + operand_->to_string() + ")";
    }
    
    std::map<std::string, std::string> get_parameter_map() const override {
        return operand_->get_parameter_map();
    }

    var evaluate_autodiff(const std::map<std::string, double>& variable_values,
                         const std::string& target_variable,
                         var target_value) const override {
        var operand_val = operand_->evaluate_autodiff(variable_values, target_variable, target_value);
        
        if (op_symbol_.find("sin") == 0) {
            return sin(operand_val);
        } else if (op_symbol_.find("cos") == 0) {
            return cos(operand_val);
        } else if (op_symbol_.find("tan") == 0) {
            return tan(operand_val);
        } else if (op_symbol_.find("exp") == 0) {
            return exp(operand_val);
        } else if (op_symbol_.find("log") == 0) {
            return log(operand_val);
        } else if (op_symbol_.find("sqrt") == 0) {
            return sqrt(operand_val);
        } else if (op_symbol_.find("pow") == 0) {
            // 从字符串中提取指数
            size_t start = op_symbol_.find('(') + 1;
            size_t end = op_symbol_.find(')');
            double exponent = std::stod(op_symbol_.substr(start, end - start));
            return pow(operand_val, exponent);
        } else if (op_symbol_ == "-") {
            return -operand_val;
        } else {
            return operand_val;
        }
    }
};

class Parameter {
private:
    std::unique_ptr<ExpressionNode> expression_;
    std::string name_;
    std::string uuid_;

public:
    // 创建常数参数（允许隐式转换）
    Parameter(double val) 
        : expression_(std::make_unique<ConstantNode>(val)), 
          uuid_(UUIDGenerator::generate()) {}

    // 创建变量参数
    explicit Parameter(const std::string& name) 
        : name_(name),
          uuid_(UUIDGenerator::generate()) {
        expression_ = std::make_unique<VariableNode>(name, uuid_);
    }

    // 从表达式创建参数（内部使用）
    Parameter(std::unique_ptr<ExpressionNode> expr) 
        : expression_(std::move(expr)), 
          uuid_(UUIDGenerator::generate()) {}

    Parameter(const Parameter& other) 
        : expression_(other.expression_->clone()), 
          name_(other.name_),
          uuid_(other.uuid_) {}  // 拷贝时保持相同的 UUID

    Parameter& operator=(const Parameter& other) {
        if (this != &other) {
            expression_ = other.expression_->clone();
            name_ = other.name_;
            uuid_ = other.uuid_;
        }
        return *this;
    }

    Parameter(Parameter&& other) noexcept
        : expression_(std::move(other.expression_)), 
          name_(std::move(other.name_)),
          uuid_(std::move(other.uuid_)) {}

    Parameter& operator=(Parameter&& other) noexcept {
        if (this != &other) {
            expression_ = std::move(other.expression_);
            name_ = std::move(other.name_);
            uuid_ = std::move(other.uuid_);
        }
        return *this;
    }

    double value(const std::map<std::string, double>& variable_values = {}) const {
        return expression_->evaluate(variable_values);
    }

    void set_value(double val) {
        expression_ = std::make_unique<ConstantNode>(val);
    }

    const ExpressionNode* get_expression() const {
        return expression_.get();
    }

    std::string get_name() const {
        return name_;
    }
    
    std::string get_uuid() const {
        return uuid_;
    }

    std::vector<std::string> get_variables() const {
        return expression_->get_variables();
    }
    
    // 获取参数映射表
    std::map<std::string, std::string> get_parameter_map() const {
        return expression_->get_parameter_map();
    }

    std::map<std::string, double> compute_gradients(const std::map<std::string, double>& variable_values) const {
        std::map<std::string, double> gradients;
        
        auto variables = get_variables();
        
        for (const auto& var_name : variables) {
            auto it = variable_values.find(var_name);
            if (it != variable_values.end()) {
                var x = it->second;
                
                // 创建 autodiff 函数：将表达式转换为 autodiff 函数
                auto f = [this, &variable_values, &var_name](var x) -> var {
                    return this->evaluate_with_autodiff(variable_values, var_name, x);
                };
                
                var y = f(x);
                gradients[var_name] = derivatives(y, wrt(x))[0];
            } else {
                gradients[var_name] = 0.0;
            }
        }
        
        return gradients;
    }

    // 运算符重载
    friend Parameter operator+(const Parameter& lhs, const Parameter& rhs) {
        auto left = lhs.expression_->clone();
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a + b; }, "+"));
    }

    friend Parameter operator+(const Parameter& lhs, double rhs) {
        auto left = lhs.expression_->clone();
        auto right = std::make_unique<ConstantNode>(rhs);
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a + b; }, "+"));
    }

    friend Parameter operator+(double lhs, const Parameter& rhs) {
        auto left = std::make_unique<ConstantNode>(lhs);
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a + b; }, "+"));
    }

    friend Parameter operator-(const Parameter& lhs, const Parameter& rhs) {
        auto left = lhs.expression_->clone();
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a - b; }, "-"));
    }

    friend Parameter operator-(const Parameter& lhs, double rhs) {
        auto left = lhs.expression_->clone();
        auto right = std::make_unique<ConstantNode>(rhs);
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a - b; }, "-"));
    }

    friend Parameter operator-(double lhs, const Parameter& rhs) {
        auto left = std::make_unique<ConstantNode>(lhs);
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a - b; }, "-"));
    }

    friend Parameter operator*(const Parameter& lhs, const Parameter& rhs) {
        auto left = lhs.expression_->clone();
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a * b; }, "*"));
    }

    friend Parameter operator*(const Parameter& lhs, double rhs) {
        auto left = lhs.expression_->clone();
        auto right = std::make_unique<ConstantNode>(rhs);
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a * b; }, "*"));
    }

    friend Parameter operator*(double lhs, const Parameter& rhs) {
        auto left = std::make_unique<ConstantNode>(lhs);
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a * b; }, "*"));
    }

    friend Parameter operator/(const Parameter& lhs, const Parameter& rhs) {
        auto left = lhs.expression_->clone();
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a / b; }, "/"));
    }

    friend Parameter operator/(const Parameter& lhs, double rhs) {
        auto left = lhs.expression_->clone();
        auto right = std::make_unique<ConstantNode>(rhs);
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a / b; }, "/"));
    }

    friend Parameter operator/(double lhs, const Parameter& rhs) {
        auto left = std::make_unique<ConstantNode>(lhs);
        auto right = rhs.expression_->clone();
        return Parameter(std::make_unique<BinaryOpNode>(
            std::move(left), std::move(right),
            [](double a, double b) { return a / b; }, "/"));
    }

    Parameter operator-() const {
        auto operand = expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return -x; }, "-"));
    }

    friend Parameter sin(const Parameter& p) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return std::sin(x); }, "sin"));
    }

    friend Parameter cos(const Parameter& p) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return std::cos(x); }, "cos"));
    }

    friend Parameter tan(const Parameter& p) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return std::tan(x); }, "tan"));
    }

    friend Parameter exp(const Parameter& p) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return std::exp(x); }, "exp"));
    }

    friend Parameter log(const Parameter& p) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return std::log(x); }, "log"));
    }

    friend Parameter sqrt(const Parameter& p) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [](double x) { return std::sqrt(x); }, "sqrt"));
    }

    friend Parameter pow(const Parameter& p, double exponent) {
        auto operand = p.expression_->clone();
        return Parameter(std::make_unique<UnaryOpNode>(
            std::move(operand),
            [exponent](double x) { return std::pow(x, exponent); }, 
            "pow(" + std::to_string(exponent) + ")"));
    }

    std::string to_string() const {
        return expression_->to_string();
    }

    static Parameter variable(const std::string& name) {
        return Parameter(name);  // 直接使用变量参数构造函数
    }
    
    // 比较操作符 - 基于 UUID
    bool operator==(const Parameter& other) const {
        return uuid_ == other.uuid_;
    }
    
    bool operator!=(const Parameter& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Parameter& other) const {
        return uuid_ < other.uuid_;
    }
    
    // 哈希支持
    struct Hash {
        std::size_t operator()(const Parameter& param) const {
            return std::hash<std::string>{}(param.uuid_);
        }
    };

private:
    // 辅助方法：使用 autodiff 重新求值表达式
    var evaluate_with_autodiff(const std::map<std::string, double>& variable_values, 
                              const std::string& target_variable, 
                              var target_value) const {
        // 递归构建 autodiff 表达式
        return expression_->evaluate_autodiff(variable_values, target_variable, target_value);
    }
};

} // namespace qsteedcpp