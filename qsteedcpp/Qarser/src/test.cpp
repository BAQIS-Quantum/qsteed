#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "AST/printer.hpp"
#include "SA/analyzer.hpp"
#include "AST/dag_converter.hpp"
#include "visualization.h"
#include "compiler.h"

std::string debug_qasm2 = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[6];
    creg meas[6];
    h q[0];
    h q[1];
    h q[2];
    barrier q[0],q[1],q[2],q[3],q[4],q[5];
    cx q[2],q[4];
    x q[3];
    cx q[2],q[3];
    ccx q[0],q[1],q[3];
    x q[0];
    x q[1];
    ccx q[0],q[1],q[3];
    x q[0];
    x q[1];
    x q[3];
    barrier q[0],q[1],q[2],q[3],q[4],q[5];
    h q[0];
    h q[1];
    h q[2];
    barrier q[0],q[1],q[2],q[3],q[4],q[5];
    measure q[0] -> meas[0];
    measure q[1] -> meas[1];
    measure q[2] -> meas[2];
    measure q[3] -> meas[3];
    measure q[4] -> meas[4];
    measure q[5] -> meas[5];
)";

std::string debug_qasm = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    qreg q[6];
    creg meas[6];
    ccx q[0],q[1],q[3];
)";

std::string debug_qasm1 = R"(
    OPENQASM 2.0;
    include "qelib1.inc";
    ccx q[0],q[1],q[3];
    qreg q[6];
    creg meas[6];
    ccx q[0],q[1],q[3];
    u1(0.1, 0.2) q[0];
    u2(0.1) q[1];
    // u3(0.1, 0.2, 0.3) q[2];
    u1(0.2+theta) q[3];
    u2(3*(0.1+pi)-2, 0.2*pi) q[3];
    CX q[0], q[1], q[2];
    CX q;

    gate cphase(lambda) a,b,b {
        u1(lambda/2) a;
        CX a,b[1];
        u1(-lambda/2) b;
        CX a,b;
        // u1(lambda/2) b;
        CX(pi/2) a,b;
        CX a;
        U(0,theta,pi/2) a;
    }
)";

void test_lexer() {
    qarser::QasmLexer lexer(debug_qasm1);
    while (!lexer.is_at_end())
    {
        lexer.next().print();
    }
}

void test_parser() {
    qarser::Parser parser(debug_qasm1);
    auto ast = parser.parse();

    qarser::AstPrinter printer;
    ast->accept(printer);
}

void test_sa() {
    qarser::Parser parser(debug_qasm1);
    auto ast = parser.parse();

    qarser::SemanticAnalyzer sa;
    sa.analyze(*ast);
}


void test_dag_converter() {
    qarser::Parser parser(debug_qasm2);
    auto ast = parser.parse();

    qarser::AstPrinter printer;
    ast->accept(printer);

    qarser::SemanticAnalyzer sa;
    sa.analyze(*ast);

    qarser::AstToDagConverter converter;
    ast->accept(converter);

    // draw_graph(converter.dag.graph);
    // std::cout << "DAGCircuit: " << std::endl;
    // std::cout << converter.dag.graph << std::endl;
}

void test_compiler() {
    qarser::QasmCompiler compiler{"../qsteedcpp/Qarser/src/test.qasm", true};
    compiler.print_source().parse().analyze();
}



int main() {
    std::cout << "---- qarser_test main function ----" << std::endl;
    // test_lexer();
    // test_parser();
    // test_sa();
    // test_dag_converter();
    test_compiler();
    return 0;
}
