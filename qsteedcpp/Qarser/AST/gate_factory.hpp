#pragma once
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include "QuantumCircuit/gates/standard_gates.h"
#include "QuantumCircuit/expression/parameter.h"

namespace qsteedcpp {
namespace qarser {

// String 2 Gate
class GateFactory {
public:
    using GateCreator = std::function<std::unique_ptr<Gate>(const std::vector<Parameter>&)>;
    
private:
    static inline std::map<std::string, GateCreator> gate_creators;
    
    static std::map<std::string, GateCreator> init_gate_creators() {
        std::map<std::string, GateCreator> creators;
        
        creators["h"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<HGate>(); 
        };
        creators["x"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<XGate>(); 
        };
        creators["y"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<YGate>(); 
        };
        creators["z"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<ZGate>(); 
        };
        creators["s"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<SGate>(); 
        };
        creators["sdg"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<SdgGate>(); 
        };
        creators["t"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<TGate>(); 
        };
        creators["tdg"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<TdgGate>(); 
        };
        // TODO: 
        // creators["id"] = [](const std::vector<Parameter>&) { 
        //     return std::make_unique<HGate>(); 
        // };
        
        // 单参数单量子比特门
        creators["rx"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("rx requires 1 parameter");
            return std::make_unique<RXGate>(params[0]);
        };
        creators["ry"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("ry requires 1 parameter");
            return std::make_unique<RYGate>(params[0]);
        };
        creators["rz"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("rz requires 1 parameter");
            return std::make_unique<RZGate>(params[0]);
        };
        creators["p"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("p requires 1 parameter");
            return std::make_unique<PhaseGate>(params[0]);
        };
        creators["u1"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("u1 requires 1 parameter");
            return std::make_unique<PhaseGate>(params[0]);  // u1是p的别名
        };
        
        // 三参数单量子比特门
        creators["u3"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 3) throw std::invalid_argument("u3 requires 3 parameters");
            return std::make_unique<U3Gate>(params[0], params[1], params[2]);
        };
        creators["u"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 3) throw std::invalid_argument("u requires 3 parameters");
            return std::make_unique<U3Gate>(params[0], params[1], params[2]);  // u是u3的别名
        };
        
        // 双参数单量子比特门
        creators["u2"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 2) throw std::invalid_argument("u2 requires 2 parameters");
            // u2(phi,lambda) = u3(pi/2, phi, lambda)
            return std::make_unique<U3Gate>(Parameter(M_PI/2), params[0], params[1]);
        };
        
        // 无参数双量子比特门
        creators["cx"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<CNOTGate>(); 
        };
        creators["CX"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<CNOTGate>(); 
        };
        creators["cnot"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<CNOTGate>(); 
        };
        creators["cz"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<CZGate>(); 
        };
        creators["swap"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<SwapGate>(); 
        };
        creators["iswap"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<iSwapGate>(); 
        };
        
        // 单参数双量子比特门
        creators["rxx"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("rxx requires 1 parameter");
            return std::make_unique<RXXGate>(params[0]);
        };
        creators["ryy"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("ryy requires 1 parameter");
            return std::make_unique<RYYGate>(params[0]);
        };
        creators["rzz"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("rzz requires 1 parameter");
            return std::make_unique<RZZGate>(params[0]);
        };
        creators["crz"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("crz requires 1 parameter");
            // 需要实现CRZGate
            throw std::runtime_error("CRZ gate not implemented yet");
        };
        creators["cu1"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("cu1 requires 1 parameter");
            // 需要实现CU1Gate
            throw std::runtime_error("CU1 gate not implemented yet");
        };
        creators["cp"] = [](const std::vector<Parameter>& params) {
            if (params.size() != 1) throw std::invalid_argument("cp requires 1 parameter");
            // 需要实现CPGate
            throw std::runtime_error("CP gate not implemented yet");
        };
        
        // 三量子比特门
        creators["ccx"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<ToffoliGate>(); 
        };
        creators["toffoli"] = [](const std::vector<Parameter>&) { 
            return std::make_unique<ToffoliGate>(); 
        };
        // TODO
        // creators["cswap"] = [](const std::vector<Parameter>&) { 
            // throw std::runtime_error("CSWAP gate not implemented yet");
        // };
        
        return creators;
    }
    
public:
    static std::unique_ptr<Gate> create_gate(const std::string& gate_name, 
                                             const std::vector<Parameter>& params) {
        // 延迟初始化
        if (gate_creators.empty()) {
            gate_creators = init_gate_creators();
        }
        
        auto it = gate_creators.find(gate_name);
        if (it == gate_creators.end()) {
            throw std::runtime_error("Unknown gate: " + gate_name);
        }
        
        return it->second(params);
    }
    
    static bool is_gate_supported(const std::string& gate_name) {
        if (gate_creators.empty()) {
            gate_creators = init_gate_creators();
        }
        return gate_creators.find(gate_name) != gate_creators.end();
    }
    
    static int get_gate_param_count(const std::string& gate_name) {
        static std::map<std::string, int> param_counts = {
            {"h", 0}, {"x", 0}, {"y", 0}, {"z", 0}, 
            {"s", 0}, {"sdg", 0}, {"t", 0}, {"tdg", 0}, {"id", 0},
            {"cx", 0}, {"CX", 0}, {"cnot", 0}, {"cz", 0}, 
            {"swap", 0}, {"iswap", 0}, {"ccx", 0}, {"toffoli", 0}, {"cswap", 0},
            
            // 单参数门
            {"rx", 1}, {"ry", 1}, {"rz", 1}, {"p", 1}, {"u1", 1},
            {"rxx", 1}, {"ryy", 1}, {"rzz", 1}, {"crz", 1}, {"cu1", 1}, {"cp", 1},
            
            // 双参数门
            {"u2", 2},
            
            // 三参数门
            {"u3", 3}, {"u", 3}, {"cu3", 3}
        };
        
        auto it = param_counts.find(gate_name);
        return (it != param_counts.end()) ? it->second : -1;
    }
    
    static int get_gate_qubit_count(const std::string& gate_name) {
        static std::map<std::string, int> qubit_counts = {
            // 单量子比特门
            {"h", 1}, {"x", 1}, {"y", 1}, {"z", 1}, 
            {"s", 1}, {"sdg", 1}, {"t", 1}, {"tdg", 1}, {"id", 1},
            {"rx", 1}, {"ry", 1}, {"rz", 1}, {"p", 1}, {"u1", 1},
            {"u2", 1}, {"u3", 1}, {"u", 1},
            
            // 双量子比特门
            {"cx", 2}, {"CX", 2}, {"cnot", 2}, {"cz", 2}, 
            {"swap", 2}, {"iswap", 2},
            {"rxx", 2}, {"ryy", 2}, {"rzz", 2}, 
            {"crz", 2}, {"cu1", 2}, {"cp", 2}, {"cu3", 2},
            
            // 三量子比特门
            {"ccx", 3}, {"toffoli", 3}, {"cswap", 3}
        };
        
        auto it = qubit_counts.find(gate_name);
        return (it != qubit_counts.end()) ? it->second : -1;
    }
};


} // namespace qarser
} // namespace qsteedcpp