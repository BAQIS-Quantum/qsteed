#pragma once
#include <string>
#include <memory>
#include <set>

namespace qsteedcpp {

enum class BinaryOpType {
    ADD,    // +
    SUB,    // -
    MUL,    // *
    DIV,    // /
    POW     // ^
};

enum class UnaryOpType {
    NEG,    // -x
    SIN,    // sin(x)
    COS,    // cos(x)
    TAN,    // tan(x)
    EXP,    // exp(x)
    LOG,    // log(x)
    SQRT    // sqrt(x)
};


class Expression {
public:
    virtual ~Expression() = default;

    // Node type enumeration
    enum class Type {
        CONSTANT,
        PARAMETER,
        BINARY_OP,
        UNARY_OP
    };

    virtual Type get_type() const = 0;
    virtual std::set<std::string> get_parameter_uuids() const = 0;
    virtual std::string to_string(bool numeric_params) const = 0;
    virtual std::shared_ptr<Expression> clone() const = 0;
    virtual double eval() const = 0;
};

} // namespace qsteedcpp
