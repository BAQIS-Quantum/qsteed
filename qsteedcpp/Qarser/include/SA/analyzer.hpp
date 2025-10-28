#pragma once
#include <memory>
#include "SA/context/analysis_context.hpp"
#include "AST/ast.hpp"
#include "analyzers/declaration_analyzer.hpp"
#include "analyzers/gate_usage_analyzer.hpp"
#include "analyzers/gate_def_analyzer.hpp"


namespace qsteedcpp {
namespace qarser {

    // qelib1.inc 标准门定义
    inline void register_qelib1_gates(SymbolTable& symbols) {
        // QE Hardware primitives
        symbols.add_gate("u3", 3, 1);  // u3(theta,phi,lambda)
        symbols.add_gate("u2", 2, 1);  // u2(phi,lambda) = u3(pi/2,phi,lambda)
        symbols.add_gate("u1", 1, 1);  // u1(lambda) = u3(0,0,lambda)
        symbols.add_gate("id", 0, 1);  // 单位门
        symbols.add_gate("u0", 1, 1);  // u0(gamma)

        // 别名
        symbols.add_gate("u", 3, 1);   // u(theta,phi,lambda) = u3的别名
        symbols.add_gate("p", 1, 1);   // p(lambda) = u1的别名

        // QE Standard Gates
        symbols.add_gate("x", 0, 1);   // Pauli-X
        symbols.add_gate("y", 0, 1);   // Pauli-Y
        symbols.add_gate("z", 0, 1);   // Pauli-Z
        symbols.add_gate("h", 0, 1);   // Hadamard
        symbols.add_gate("s", 0, 1);   // S门 = u1(pi/2)
        symbols.add_gate("sdg", 0, 1); // S门的共轭
        symbols.add_gate("t", 0, 1);   // T门 = u1(pi/4)
        symbols.add_gate("tdg", 0, 1); // T门的共轭

        // Standard rotations
        symbols.add_gate("rx", 1, 1);  // 绕X轴旋转
        symbols.add_gate("ry", 1, 1);  // 绕Y轴旋转
        symbols.add_gate("rz", 1, 1);  // 绕Z轴旋转

        // QE Standard User-Defined Gates
        symbols.add_gate("cx", 0, 2);   // 受控-X门 (CNOT)
        symbols.add_gate("cy", 0, 2);   // 受控-Y门
        symbols.add_gate("cz", 0, 2);   // 受控-Z门
        symbols.add_gate("ch", 0, 2);   // 受控-H门
        symbols.add_gate("ccx", 0, 3);  // Toffoli门
        symbols.add_gate("cswap", 0, 3); // Fredkin门
        symbols.add_gate("swap", 0, 2);  // SWAP门
        
        // Controlled rotations
        symbols.add_gate("crx", 1, 2);  // 受控-RX门
        symbols.add_gate("cry", 1, 2);  // 受控-RY门
        symbols.add_gate("crz", 1, 2);  // 受控-RZ门
        symbols.add_gate("cu1", 1, 2);  // 受控-U1门
        symbols.add_gate("cp", 1, 2);   // 受控相位门（cu1的别名）
        symbols.add_gate("cu3", 3, 2);  // 受控-U3门
    }

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
                    case Statement::Kind::INCLUDE: {
                        auto* include_stmt = static_cast<Include*>(stmt.get());
                        if (include_stmt->filename == "qelib1.inc") {
                            register_qelib1_gates(context.get_symbols());
                        } else {
                            context.add_error(include_stmt->line, 
                                "Cannot find include file: " + include_stmt->filename);
                        }
                        break;
                    }
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

        void init_analyzers() {
            declaration_analyzer = std::make_unique<DeclarationAnalyzer>(context);
            gate_analyzer = std::make_unique<GateAnalyzer>(context);
            gate_def_analyzer = std::make_unique<GateDefAnalyzer>(context);
        }

    };


        

}; // namespace qarser
}; // namespace qsteedcpp