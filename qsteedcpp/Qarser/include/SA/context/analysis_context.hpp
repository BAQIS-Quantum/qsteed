#pragma once
#include "symbol.hpp"
#include "SA/error/error.hpp"

namespace qarser {

    class AnalysisContext {
    private:
        SymbolTable symbols;
        ErrorCollector errors;
        
    public:
        SymbolTable& get_symbols() { 
            return symbols; 
        }
        ErrorCollector& get_errors() { 
            return errors; 
        }


        void add_error(int line, const std::string& message) {
            errors.add_error(line, message);
        }

        void init_builtins() {
            symbols.add_gate("U", 3, 1);
            symbols.add_gate("CX", 0, 2);
        }
    };

}