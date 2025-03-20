#pragma once
#include "base_analyzer.hpp"


namespace qarser {

    class DeclarationAnalyzer : public BaseAnalyzer {
    public:
        DeclarationAnalyzer(AnalysisContext& context) 
            : BaseAnalyzer(context) {}


        void visit(QRegister& qreg) override {
            if (qreg.size <= 0) {
                context.add_error(qreg.line, "Invalid quantum register size");
                return;
            }
            if (!context.get_symbols().add_qreg(qreg.name, qreg.size)) {
                context.add_error(qreg.line, "Redefinition of quantum register '" + qreg.name + "'");
            }
        }

        void visit(CRegister& creg) override {
            if (creg.size <= 0) {
                context.add_error(creg.line, "Invalid classical register size");
                return;
            }
            if (!context.get_symbols().add_creg(creg.name, creg.size)) {
                context.add_error(creg.line, "Redefinition of classical register '" + creg.name + "'");
            }
        }
    };

}; // namespace qarser