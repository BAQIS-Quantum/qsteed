#pragma once
#include "AST/ast.hpp"
#include "AST/visitor.hpp"
#include "SA/analyzer.hpp"
#include "parser.h"
#include "dag.h"



namespace qarser {

    class AstToDagConverter : public BaseVisitor {
    public:
        DAGCircuit dag;
        std::unordered_map<std::string, int> qreg;
        std::unordered_map<std::string, int> creg;
        

    public:
        void visit(Program& program) {
            dag = DAGCircuit();
            last_qubit_node.clear();
            
            for (auto& stmt : program.statements) {
                stmt->accept(*this);
            }
        }

        void visit(QRegister& quantum_register) {
            qreg[quantum_register.name] = quantum_register.size;
        }

        void visit(CRegister& classical_register) {
            creg[classical_register.name] = classical_register.size;
        }

        void visit(Gate& gate) {
            std::vector<qubit_t> qubit_pos = get_gate_pos(gate);
            if (qubit_pos.empty()) {
                throw std::runtime_error("No qubits found for gate: " + gate.name);
            }

            InstructionNode node(gate.name, qubit_pos);
            dag.add_instruction_node_end(node);
        }


    private:
        std::vector<int> last_qubit_node;
        std::unordered_map<std::string, int> qubit_offset;

        void precompute_offsets() {
            int offset = 0;
            for (const auto& [name, size] : this->qreg) {
                qubit_offset[name] = offset;
                offset += size;
            }
        }

        int get_qubit_global_pos(const std::string& qreg_name, int index) {
            if (qubit_offset.empty() && !qreg.empty()) {
                precompute_offsets();
            }
            
            auto it = qubit_offset.find(qreg_name);
            if (it == qubit_offset.end()) {
                return -1;
            }
            return it->second + index;
        }



        std::vector<qubit_t> get_gate_pos(const Gate& gate) {
            std::vector<qubit_t> qubit_pos;
            for (const auto& qubit : gate.qubits) {
                auto it = qreg.find(qubit.name);
                if (it == qreg.end()) {
                    throw std::runtime_error("Qubit register not found: " + qubit.name);
                }

                if (qubit.isRefWholeRegister()) {
                    for (int i = 0; i < it->second; ++i) {
                        qubit_pos.push_back(get_qubit_global_pos(qubit.name, i));
                    }
                } else {
                    qubit_pos.push_back(get_qubit_global_pos(qubit.name, qubit.index));
                }
            }
            return qubit_pos;
        }


    };


    inline DAGCircuit qasm_to_dag(const std::string& src) {
        qarser::Parser parser{src};
        auto ast = parser.parse();

        // qarser::AstPrinter printer;
        // ast->accept(printer);

        qarser::SemanticAnalyzer sa;
        sa.analyze(*ast);

        qarser::AstToDagConverter converter;
        ast->accept(converter);
        
        return converter.dag;
    }

};