#pragma once
#include <unordered_map>
#include <string>



namespace qarser {

    class GateScopeSymbol {
    public:
        std::string name;
        
        explicit GateScopeSymbol(const std::string& name) : name(name) {}
        virtual ~GateScopeSymbol() = default;
    };


    class ParamSymbol : public GateScopeSymbol {
    public:
        explicit ParamSymbol(const std::string& name) 
            : GateScopeSymbol(name) {}
    };

    class QubitArgSymbol : public GateScopeSymbol {
    public:
        explicit QubitArgSymbol(const std::string& name) 
            : GateScopeSymbol(name) {}
    };



    class GateScope {
    private:
        std::unordered_map<std::string, ParamSymbol> params;
        std::unordered_map<std::string, QubitArgSymbol> qargs;

    public:
        bool exists(const std::string& name) const {
            return  params.find(name) != params.end() || 
                    qargs.find(name) != qargs.end();
        }

        bool add_param(const std::string& name) {
            if (exists(name)) {
                return false;
            }
            return params.emplace(name, ParamSymbol(name)).second;
        }

        bool add_qubit(const std::string& name) {
            if (exists(name)) {
                return false;
            }
            return qargs.emplace(name, QubitArgSymbol(name)).second;
        }

        const ParamSymbol* lookup_param(const std::string& name) const {
            auto it = params.find(name);
            return it != params.end() ? &it->second : nullptr;
        }

        const QubitArgSymbol* lookup_qubit(const std::string& name) const {
            auto it = qargs.find(name);
            return it != qargs.end() ? &it->second : nullptr;
        }
    };

}