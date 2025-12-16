#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include "QuantumCircuit/expression/expr.h"

using qubit_t = int;
using cbit_t = int;
using node_pos_t = uint64_t;
using edge_pos_t = std::pair<node_pos_t, node_pos_t>;


struct InstructionNode
{
public:
    std::string name = "NoName";
    std::vector<qubit_t> qubit_pos = {};
    std::vector<qsteedcpp::Expr> paras; // Unified parameter representation

    // for gate in [Delay,XYResonance]
    std::optional<int> duration;
    std::optional<std::string> unit;

public:
    InstructionNode() {}

    InstructionNode(const std::string& name)
        : name(name) {}

    InstructionNode(const std::string& name, const std::vector<qubit_t>& qubit_pos)
        : name(name), qubit_pos(qubit_pos) {}
    
    InstructionNode(const std::string& name, const qubit_t qubit_pos)
        : name(name), qubit_pos({qubit_pos}) {}

    InstructionNode(const std::string& name,
                    const std::vector<qubit_t>& qubit_pos,
                    const std::vector<qsteedcpp::Expr>& paras,
                    const std::optional<int>& duration = std::nullopt,
                    const std::optional<std::string>& unit = std::nullopt)
        : name(name), qubit_pos(qubit_pos), paras(paras), duration(duration), unit(unit) {}

    friend std::ostream& operator<< (std::ostream& os, const InstructionNode& node);
};



struct MeasureNode : public InstructionNode
{
public:
    std::vector<cbit_t> classic_pos;

public:
    MeasureNode()
        : InstructionNode("measure") {
    }

    MeasureNode(std::vector<int> qubit_pos, std::vector<int> classic_pos)
        : InstructionNode("measure", qubit_pos), classic_pos(classic_pos) {
    }
};