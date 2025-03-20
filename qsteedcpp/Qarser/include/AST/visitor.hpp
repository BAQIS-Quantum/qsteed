#pragma once

namespace qarser {

    class Program;
    class Include;
    class QRegister;
    class CRegister;
    class Gate;
    class Measure;
    class Reset;
    class Barrier;
    class GateDef;

    class NumberExpr;
    class IdentifierExpr;
    class UnaryExpr;
    class BinaryExpr;



    class AstVisitor {
    public: 
        virtual void visit(Program& program) = 0;
        virtual void visit(Include& include) = 0;
        virtual void visit(QRegister& qreg) = 0;
        virtual void visit(CRegister& creg) = 0;
        virtual void visit(Gate& gate) = 0;
        virtual void visit(Measure& measure) = 0;
        virtual void visit(Reset& reset) = 0;
        virtual void visit(Barrier& barrier) = 0;
        virtual void visit(GateDef& gate_def) = 0;

        virtual void visit(NumberExpr& expr) = 0;
        virtual void visit(IdentifierExpr& expr) = 0;
        virtual void visit(UnaryExpr& expr) = 0;
        virtual void visit(BinaryExpr& expr) = 0;

        virtual ~AstVisitor() = default;
    };



    class BaseVisitor : public AstVisitor {
    public:
        void visit(Program& program) override {}
        void visit(Include& include) override {}
        void visit(QRegister& qreg) override {}
        void visit(CRegister& creg) override {}
        void visit(Gate& gate) override {}
        void visit(Measure& measure) override {}
        void visit(Reset& reset) override {}
        void visit(Barrier& barrier) override {}
        void visit(GateDef& gate_def) override {}

        void visit(NumberExpr& expr) override {}
        void visit(IdentifierExpr& expr) override {}
        void visit(UnaryExpr& expr) override {}
        void visit(BinaryExpr& expr) override {}
    };



}; // namespace qarser