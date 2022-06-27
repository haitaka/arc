#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_set>
#include <memory>

namespace ast {
    class Elem {
    public:
        virtual ~Elem() = default;
        virtual void print(std::ostream & out) const = 0;
    };

    class Expression : public Elem {
    public:
        class Visitor;
        virtual void accept(Visitor & visitor) = 0;
    };

    class AssignableTo : public Expression {};

    class Statement : public Elem {
    public:
        class Visitor;
        virtual void accept(Visitor & visitor) = 0;
    };

    class NewObject : public Expression {
    public:
        explicit NewObject(std::string  name);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        std::string name;
    };

    class Var : public AssignableTo {
    public:
        explicit Var(std::string name);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        std::string const name;
    };

    class SelectField : public AssignableTo {
    public:
        SelectField(std::unique_ptr<AssignableTo> && obj, std::string name);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        std::unique_ptr<AssignableTo> const obj;
        std::string const name;
    };

    class Assign : public Statement {
    public:
        Assign(std::unique_ptr<AssignableTo> && to, std::unique_ptr<Expression> && from, bool isWeak);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        bool const isWeak;
        std::unique_ptr<AssignableTo> const to;
        std::unique_ptr<Expression> const from;
    };

    class EndOfLife : public Statement {
    public:
        explicit EndOfLife(std::string var);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        std::string const varName;
    };

    class NewThread : public Statement {
    public:
        explicit NewThread(std::vector<std::unique_ptr<Statement>> && body);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        std::vector<std::unique_ptr<Statement>> const body;
        std::unordered_set<std::string> usedVars;
    };

    class Sleep : public Statement {
    public:
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    };

    class Sleepr : public Statement {
    public:
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    };

    class Dump : public Statement {
    public:
        explicit Dump(std::unique_ptr<Expression> expr);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;

        std::unique_ptr<Expression> expr;
    };

    class Statement::Visitor  {
    public:
        virtual void visitAssign(Assign & assign);
        virtual void visitNewThread(NewThread & newThread);
        virtual void visitSleep(Sleep & sleep);
        virtual void visitSleepr(Sleepr & sleepr);
        virtual void visitDump(Dump & dump);
        virtual void visitEndOfLife(EndOfLife & endOfLife);
        virtual void visitStatement(Statement & stat);
    };

    class Expression::Visitor  {
    public:
        virtual void visitNewObject(NewObject & newObject);
        virtual void visitVar(Var & var);
        virtual void visitSelectField(SelectField & selectField);
        virtual void visitExpression(Expression & expr);
    };
}
