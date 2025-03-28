#pragma once
#include <vector>
#include <memory>
#include "SA/context/analysis_context.hpp"

#include "analyzers/declaration_analyzer.hpp"
#include "analyzers/gate_usage_analyzer.hpp"
#include "analyzers/gate_def_analyzer.hpp"



namespace qarser {

    class SemanticAnalyzer {
    private:
        AnalysisContext context;
        

        std::unique_ptr<DeclarationAnalyzer> declaration_analyzer;
        std::unique_ptr<GateAnalyzer> gate_analyzer;
        std::unique_ptr<GateDefAnalyzer> gate_def_analyzer;

    public:
        SemanticAnalyzer() {
            init_builtins();
            init_analyzers();
        }


        void analyze(Program& program) {
            for (const auto& stmt : program.statements) {
                switch (stmt->kind()) {
                    case Statement::Kind::INCLUDE:
                        init_qelib1();
                        break;
                    case Statement::Kind::QREG:
                        stmt->accept(*declaration_analyzer);
                        break;
                    case Statement::Kind::CREG:
                        stmt->accept(*declaration_analyzer);
                        break;
                    case Statement::Kind::GATE:
                        stmt->accept(*gate_analyzer);
                        break;
                    case Statement::Kind::GATE_DEF:
                        stmt->accept(*gate_def_analyzer);
                        break;
                   default:
                        break;
                }
            }
            context.get_errors().report();
        }

        
    private:
        void init_builtins() {
            context.get_symbols().add_gate("U", 3, 1);
            context.get_symbols().add_gate("CX", 0, 2);
        }

        void init_qelib1() {
            // include qelib1.inc
            // QE Hardware primitives
            context.get_symbols().add_gate("u3", 3, 1);
            context.get_symbols().add_gate("u2", 2, 1);
            context.get_symbols().add_gate("u1", 1, 1);
            context.get_symbols().add_gate("id", 0, 1);

            // QE Standard Gates
            context.get_symbols().add_gate("x", 0, 1);   // Pauli-X
            context.get_symbols().add_gate("y", 0, 1);   // Pauli-Y
            context.get_symbols().add_gate("z", 0, 1);   // Pauli-Z
            context.get_symbols().add_gate("h", 0, 1);   // Hadamard
            context.get_symbols().add_gate("s", 0, 1);   // S门
            context.get_symbols().add_gate("sdg", 0, 1); // S门的共轭
            context.get_symbols().add_gate("t", 0, 1);   // T门
            context.get_symbols().add_gate("tdg", 0, 1); // T门的共轭

            // Standard rotations
            context.get_symbols().add_gate("rx", 1, 1);  // 绕X轴旋转
            context.get_symbols().add_gate("ry", 1, 1);  // 绕Y轴旋转
            context.get_symbols().add_gate("rz", 1, 1);  // 绕Z轴旋转

            // QE Standard User-Defined Gates
            context.get_symbols().add_gate("cx", 0, 2);   // 受控-X门
            context.get_symbols().add_gate("cy", 0, 2);   // 受控-Y门
            context.get_symbols().add_gate("cz", 0, 2);   // 受控-Z门
            context.get_symbols().add_gate("ch", 0, 2);   // 受控-H门
            context.get_symbols().add_gate("ccx", 0, 3);  // Toffoli门
            context.get_symbols().add_gate("crz", 1, 2);  // 受控-RZ门
            context.get_symbols().add_gate("cu1", 1, 2);  // 受控-U1门
            context.get_symbols().add_gate("cu3", 3, 2);  // 受控-U3门
        }



        void init_analyzers() {
            declaration_analyzer = std::make_unique<DeclarationAnalyzer>(context);
            gate_analyzer = std::make_unique<GateAnalyzer>(context);
            gate_def_analyzer = std::make_unique<GateDefAnalyzer>(context);
        }

    };


        

}; // namespace qarser