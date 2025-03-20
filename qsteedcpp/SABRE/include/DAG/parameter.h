#pragma once
#include <string>

enum class ParameterType {
    None,
    DOUBLE,
    PARAMETER,
    PARAMETER_EXPRESSION,
};

enum class OperatorType {
    ADD,
    SUB,
    MUL,
    TRUEDIV,
    FLOORDIV,
    POW,
    SIN,
    COS,
    TAN,
    EXP,
    LOG,
    SQRT,
    ABS,
    NEG,
    CONST,
};


class ParameterExpression
{
public:
    std::vector<double> operands = {};
    std::vector<OperatorType> operators = {};
public:
    ParameterExpression() {}

};


class Parameter : public ParameterExpression
{
public:
    ParameterType ptype = ParameterType::None;
    std::string name = "NoName";
    double value = 0.0;

    bool tunable = false;

public:
    Parameter() {}

    // For Double Version Parameter
    Parameter(ParameterType ptype, double value)
        :  ptype(ptype), value(value) {}

    Parameter(ParameterType ptype, const std::string& name, double value)
        :  ptype(ptype), name(name), value(value) {}
};