#pragma once    
#include "AST/visitor.hpp"
#include "SA/context/analysis_context.hpp"

namespace qarser {

    class BaseAnalyzer : public BaseVisitor {
    protected:
        AnalysisContext& context;

    protected:
        explicit BaseAnalyzer(AnalysisContext& context) 
            : context(context) {}
    };

}; 