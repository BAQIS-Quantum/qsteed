#pragma once    
#include "AST/ast.hpp"
#include "SA/context/symbol.hpp"
#include "SA/context/analysis_context.hpp"
#include "SA/error/error.hpp"

namespace qarser {

    class BaseAnalyzer : public BaseVisitor {
    protected:
        AnalysisContext& context;

    protected:
        explicit BaseAnalyzer(AnalysisContext& context) 
            : context(context) {}

        
    };

}; 