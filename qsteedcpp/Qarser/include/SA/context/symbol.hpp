#pragma once
#include <memory>

namespace qarser {
    enum class SymbolType {
        QREG,
        CREG,
        GATE,
    };

    inline std::string symbolTypeToString(SymbolType type) {
        switch (type) {
            case SymbolType::QREG: return "QReg";
            case SymbolType::CREG: return "CReg";
            case SymbolType::GATE: return "Gate";
        }
        return "Unknown";
    }


    class NameManager {
    private:
        std::unordered_map<std::string, SymbolType> used_names;
    
    public:
        bool add_name(const std::string& name, SymbolType type) {
            return used_names.emplace(name, type).second;
        }
    
        bool exists(const std::string& name) const {
            return used_names.find(name) != used_names.end();
        }
    };


    class Symbol {
    public:
        std::string name;

        Symbol(const std::string& name) 
            : name(name) {}

        virtual ~Symbol() = default;

        virtual SymbolType type() const = 0;
    };


    class QRegisterSymbol : public Symbol {
    public:
        int size;

    public:
        QRegisterSymbol(const std::string& name, int size) 
            : Symbol(name), size(size) {}
        
        SymbolType type() const override {
            return SymbolType::QREG;
        }
    };


    class CRegisterSymbol : public Symbol {
    public:
        int size;

    public:
        CRegisterSymbol(const std::string& name, int size) 
            : Symbol(name), size(size) {}
        
        SymbolType type() const override {
            return SymbolType::CREG;
        }
    };


    class GateSymbol : public Symbol {
    public:
        int num_params;
        int num_qubits;

    public:
        GateSymbol(const std::string& name, int num_params, int num_qubits) 
            :   Symbol(name), 
                num_params(num_params),
                num_qubits(num_qubits) {} 

        SymbolType type() const override {
            return SymbolType::GATE;
        }
    };



    class SymbolTable {
    private:
        NameManager name_manager;
        std::unordered_map<std::string, QRegisterSymbol> qregs;
        std::unordered_map<std::string, CRegisterSymbol> cregs;
        std::unordered_map<std::string, GateSymbol> gates;

    public:
        SymbolTable() = default;

        bool exists(const std::string& name) const {
            return name_manager.exists(name);
        }

        bool add_qreg(const std::string& name, int size) {
            if (!name_manager.add_name(name, SymbolType::QREG)) {
                return false;
            }
            return qregs.emplace(name, QRegisterSymbol(name, size)).second;
        }

        bool add_creg(const std::string& name, int size) {
            if (!name_manager.add_name(name, SymbolType::CREG)) {
                return false;
            }
            return cregs.emplace(name, CRegisterSymbol(name, size)).second;
        }

        bool add_gate(const std::string& name, int num_params, int num_qubits) {
            if (!name_manager.add_name(name, SymbolType::GATE)) {
                return false;
            }
            return gates.emplace(name, GateSymbol(name, num_params, num_qubits)).second;
        }


        const QRegisterSymbol* lookup_qreg(const std::string& name) const {
            auto it = qregs.find(name);
            return it != qregs.end() ? &it->second : nullptr;
        }
    
        const CRegisterSymbol* lookup_creg(const std::string& name) const {
            auto it = cregs.find(name);
            return it != cregs.end() ? &it->second : nullptr;
        }
    
        const GateSymbol* lookup_gate(const std::string& name) const {
            auto it = gates.find(name);
            return it != gates.end() ? &it->second : nullptr;
        }

        size_t get_register_size(const std::string& name) const {
            if (auto qreg = lookup_qreg(name)) {
                return qreg->size;
            }
            if (auto creg = lookup_creg(name)) {
                return creg->size;
            }
            return 0;
        }
    };


}; // namespace qarser