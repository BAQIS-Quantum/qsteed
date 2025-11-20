#pragma once
#include <fstream>
#include <sstream>
#include "preprocess.hpp"
#include "SA/analyzer.hpp"
#include "AST/dag_converter.hpp"
#include "parser.h"

namespace qsteedcpp {
namespace qarser {
    inline std::string load_qasm_from_file(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filepath);
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }



    class QasmCompiler {
    private:
        std::string source;
        std::unique_ptr<Program> ast = nullptr;
        std::unique_ptr<Parser> parser;
        SemanticAnalyzer sa;
        AstToDagConverter converter;

        enum class CompilerStage {
            INITIAL,
            PARSED,
            ANALYZED,
            CONVERTED
        } stage = CompilerStage::INITIAL;


    
    public:
        QasmCompiler(const std::string& source, bool load_from_file = false) {
            this->source = load_from_file ? load_qasm_from_file(source) : source;
        }
        
        QasmCompiler& parse() {
            parser = std::make_unique<Parser>(source);
            ast = parser->parse();

            return *this;
        }
        
        QasmCompiler& analyze() {
            if (!ast) {
                throw std::runtime_error("AST not available. Call parse() first!");
            } 

            sa.analyze(*ast);
            stage = CompilerStage::PARSED;
            return *this;
        }
        
        QasmCompiler& convert_to_dag() {
            if (!ast) {
                throw std::runtime_error("AST not available. Call parse() first!");
            }
            ast->accept(converter);
            stage = CompilerStage::CONVERTED;
            return *this;
        }

        QasmCompiler& load_from_file(const std::string& filepath) {
            source = load_qasm_from_file(filepath);
            return *this;
        }
        
        QasmCompiler& print_source() {
            std::cout << source << std::endl;
            return *this;
        }
       
        DAGCircuit get_dag() {
            if (stage < CompilerStage::CONVERTED) {
                throw std::runtime_error("No exisit dag. Call convert_to_dag first!");
            }
            return converter.dag;
        } 
    };

    inline DAGCircuit qasm_to_dag(const std::string& source, bool from_file = false) {
        QasmCompiler qcompiler(source, from_file);
        return qcompiler.parse()
                    .analyze()
                    .convert_to_dag()
                    .get_dag();
    }

};
}; // namespace qsteedcpp