#pragma once    
#include "Qarser/AST/visitor.hpp"
#include "Qarser/SA/context/analysis_context.hpp"

namespace qsteedcpp {
namespace qarser {

    class BaseAnalyzer : public BaseVisitor {
    protected:
        AnalysisContext& context;

    protected:
        explicit BaseAnalyzer(AnalysisContext& context) 
            : context(context) {}
    };

}; 
}; // namespace qsteedcpp