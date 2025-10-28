#pragma once    
#include "base_analyzer.hpp"
#include "AST/gate.hpp"


namespace qsteedcpp {
namespace qarser {


    class GateAnalyzer : public BaseAnalyzer {
    public:
        GateAnalyzer(AnalysisContext& context) 
            : BaseAnalyzer(context) {}


        void visit(Gate& gate) override {
            const GateSymbol* symbol = context.get_symbols().lookup_gate(gate.name);
            if (!symbol) {
                context.add_error(gate.line, "Gate '" + gate.name + "' not declared");
                return;
            }

            // Check params count
            if (gate.params.size() != symbol->num_params) {
                context.add_error(gate.line, 
                    "Gate '" + gate.name + "' expects " + 
                    std::to_string(symbol->num_params) + " parameters, got " +
                    std::to_string(gate.params.size()));
                return;
            }


            auto expanded_qubits = expand_register_refs(gate.qubits);

            // Check qubit count 
            if (expanded_qubits.size() != symbol->num_qubits) {
                context.add_error(gate.line, 
                    "Gate '" + gate.name + "' expects " + 
                    std::to_string(symbol->num_qubits) + " qubits, got " +
                    std::to_string(expanded_qubits.size()));
                return;
            }

            // Check register refs legality
            for (const auto& ref : expanded_qubits) {
                if (!context.get_symbols().lookup_qreg(ref.name)) {
                    context.add_error(gate.line, "register '" + ref.name + "' not declared");
                    return;
                }
                if (ref.index >= context.get_symbols().get_register_size(ref.name)) {
                    context.add_error(gate.line, "register '" + ref.name + "' index out of range");
                    return;
                }
            }
       }


    private:

        /**
         * @brief 展开寄存器引用列表，将整个寄存器引用展开为单比特引用
         * 
         * 例如:
         * 给定寄存器声明:
         *     QREG qa[3], qb[2]
         * 输入引用列表:
         *     {qa[0], qb[-1], qa[2]}
         * 展开后:
         *     {qa[0], qb[0], qb[1], qa[2]}
         * 
         * @param ref 原始寄存器引用列表
         * @return std::vector<RegisterRef> 展开后的引用列表
         * 
         * @note 必须在寄存器声明之后使用，否则无法获取正确的寄存器大小
         * @note 如果引用的寄存器未声明，可能导致未定义行为
         */
        std::vector<RegisterRef> expand_register_refs(const std::vector<RegisterRef>& ref) {
            std::vector<RegisterRef> expanded;
            for (const auto& r : ref) {
                if (r.index == -1) {
                    size_t size = context.get_symbols().get_register_size(r.name);
                    for (int i = 0; i < size; ++i) {
                        expanded.push_back(RegisterRef(r.name, i));
                    }
                } 
                else
                    expanded.push_back(r);
            }
            return expanded;
        }
 

    };

}; // namespace qarser
}; // namespace qsteedcpp