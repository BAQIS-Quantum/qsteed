#pragma once
#include "base_analyzer.hpp"
#include "SA/context/gate_def_context.hpp"
#include "AST/gate.hpp"


namespace qsteedcpp {
namespace qarser {

    class ParamExpressionValidator : public BaseVisitor {
        private:
            AnalysisContext& context;
            GateScope& gate_scope;
        
        public:
            ParamExpressionValidator(AnalysisContext& context, GateScope& scope)
                : gate_scope(scope), context(context){}
        
            void visit(IdentifierExpr& id) override {
                if (!gate_scope.lookup_param(id.name)) {
                    context.add_error(id.line, "Parameter '" + id.name + "' not declared in gate definition");
                }
            }
        
            void visit(BinaryExpr& expr) override {
                expr.left->accept(*this);
                expr.right->accept(*this);
            }
            void visit(UnaryExpr& expr) override {
                expr.operand->accept(*this);
            }
        };


    class GateDefBodyAnalyzer : public BaseAnalyzer {
    private:
        GateScope& gate_scope;

    public:
        GateDefBodyAnalyzer(AnalysisContext& context, GateScope& gate_scope)
            : BaseAnalyzer(context), gate_scope(gate_scope) {}

        void visit(Gate& gate) {
            auto* gate_symbol = context.get_symbols().lookup_gate(gate.name);
            if (!gate_symbol) {
                context.add_error(gate.line, "Undefined gate '" + gate.name + "'");
                return;
            }

            if (gate.params.size() != gate_symbol->num_params) {
                context.add_error(gate.line,
                    "Gate '" + gate.name + "' expects " +
                    std::to_string(gate_symbol->num_params) + " parameters, got " +
                    std::to_string(gate.params.size()));
                return;
            }

            if (gate.qubits.size() != gate_symbol->num_qubits) {
                context.add_error(gate.line, 
                    "Gate '" + gate.name + "' expects " + 
                    std::to_string(gate_symbol->num_qubits) + " qubits, got " +
                    std::to_string(gate.qubits.size()));
                return;
            }


            for (const auto& param : gate.params) {
                ParamExpressionValidator validator(context, gate_scope);
                param->accept(validator);
            }


            for (const auto& qubit : gate.qubits) {
                if (qubit.index != -1) {
                    context.add_error(gate.line,  
                        "The arguments in qargs cannot be indexed within the body of the gate definition."
                    );
                }
                if (gate_scope.lookup_qubit(qubit.name) == nullptr) {
                    context.add_error(gate.line, "Undefined qubit '" + qubit.name + "'");
                }
            }
        }

    };





    class GateDefAnalyzer : public BaseAnalyzer {
    private:
        std::unique_ptr<GateDefBodyAnalyzer> body_analyzer = nullptr;
        std::unique_ptr<GateScope> gate_scope = nullptr;

    public:
        GateDefAnalyzer(AnalysisContext& context) 
            : BaseAnalyzer(context) {}


        void visit(GateDef& gate_def) override {
            if (!context.get_symbols().add_gate(gate_def.name, gate_def.params.size(), gate_def.qubits.size())) {
                context.add_error(gate_def.line, "Redefinition of gate '" + gate_def.name + "'");
                return;
            }


            gate_scope = std::make_unique<GateScope>();
            body_analyzer = std::make_unique<GateDefBodyAnalyzer>(context, *gate_scope);

            for (const auto& qubit : gate_def.qubits) {
                if (!gate_scope->add_qubit(qubit.name)) {
                    context.add_error(gate_def.line, "Name '" + qubit.name + "' already used in gate definition");
                }
            }

            for (const auto& param : gate_def.params) {
                if (!gate_scope->add_param(param)) {
                    context.add_error(gate_def.line, "Name '" + param + "' already used in gate definition");
                }
            }


            for (const auto& stmt : gate_def.body) {
                stmt->accept(*body_analyzer);
            }
        }

    };



}; // namespace qarser
}; // namespace qsteedcpp