#pragma once
#include <vector>
#include <string>
#include "sabre_core.h"
#include "parameter.h"

using namespace sabre;

struct InstructionNode 
{
/*
    A struct to represent a quantum gate or operation in the DAG graph. 
 
    Attributes:
    -----------
    name : std::string
        The name of the gate.
    pos : std::vector<qubit_t>
        The qubit used of the gate.
    paras : List[float]
        The parameters of the gate.
    duration : 
        The duration of the gate. Only applicable for certain gates.
    unit : 
        The unit of the duration. Only applicable for certain gates.
*/
public:
    std::string name = "NoName"; 
    std::vector<qubit_t> qubit_pos = {};

    std::vector<double> paras = {};
    std::vector<Parameter> parameters;

    // for gate in [Delay,XYResonance] in quafu
    int duration = 0;   
    std::string unit = "";

public:

    InstructionNode() {}

    InstructionNode(const std::string& name) 
        : name(name) {}

    InstructionNode(const std::string& name, const std::vector<qubit_t> qubit_pos) 
        : name(name), qubit_pos(qubit_pos) {}

    InstructionNode(const std::string& name, const qubit_t qubit_pos) 
        : name(name), qubit_pos({qubit_pos}) {}


    InstructionNode(const std::string& name, 
                    const int qubit_pos, 
                    const std::vector<Parameter> parameter, 
                    const int duration, 
                    const std::string unit) 
        : name(name), qubit_pos({qubit_pos}), parameters(parameter), duration(duration), unit(unit) {}

    InstructionNode(const std::string& name, 
                    const std::vector<int> qubit_pos, 
                    const std::vector<Parameter> parameter, 
                    const int duration, 
                    const std::string unit) 
        : name(name), qubit_pos(qubit_pos), parameters(parameter), duration(duration), unit(unit) {}


    InstructionNode(const std::string& name, 
                    const int qubit_pos, 
                    const std::vector<double> paras, 
                    const int duration, 
                    const std::string unit) 
        : name(name), qubit_pos({qubit_pos}), paras(paras), duration(duration), unit(unit) {}

    InstructionNode(const std::string& name, 
                    const std::vector<int> qubit_pos, 
                    const std::vector<double> paras, 
                    const int duration, 
                    const std::string unit) 
        : name(name), qubit_pos(qubit_pos), paras(paras), duration(duration), unit(unit) {}

    InstructionNode(const std::string& name, 
                    const int qubit_pos, 
                    const double paras, 
                    const int duration, 
                    const std::string unit) 
        : name(name), qubit_pos({qubit_pos}), paras({paras}), duration(duration), unit(unit) {}

    InstructionNode(const std::string& name, 
                    const std::vector<int> qubit_pos, 
                    const double paras, 
                    const int duration, 
                    const std::string unit) 
        : name(name), qubit_pos(qubit_pos), paras({paras}), duration(duration), unit(unit) {}




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