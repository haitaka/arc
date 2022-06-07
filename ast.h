#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>

// FIXME
#define private public

namespace ast {
    class Elem {
    public:
        virtual void print(std::ostream & out) const = 0;
    };

    // TODO mark abstracts ?
    class Expression : public Elem {
    public:
        class Visitor;
        virtual void accept(Visitor & visitor) = 0;
    };

    class AssignableTo : public Expression {
    public:
    };

    class Statement : public Elem {
    public:
        class Visitor; // TODO const visitors?
        virtual void accept(Visitor & visitor) = 0;
    };

    class NewObject : public Expression {
    public:
        NewObject() = default;
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    };

    class Var : public AssignableTo {
    public:
        explicit Var(std::string const & name);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    private:
        std::string name;
    };

    class SelectField : public AssignableTo {
    public:
        SelectField(std::unique_ptr<AssignableTo> && obj, std::string const & name);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    private:
        std::unique_ptr<AssignableTo> obj;
        std::string name;
    };

    class AssignStrong : public Statement {
    public:
        AssignStrong(std::unique_ptr<AssignableTo> && to, std::unique_ptr<Expression> && from);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    private:
        std::unique_ptr<AssignableTo> to;
        std::unique_ptr<Expression> from;
    };

    class AssignWeak : public Statement {
    public:
        AssignWeak(std::unique_ptr<AssignableTo> && to, std::unique_ptr<Expression> && from);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    private:
        std::unique_ptr<AssignableTo> to;
        std::unique_ptr<Expression> from;
    };

    class EndOfLife : public Statement {
    public:
        explicit EndOfLife(std::string const & var);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    private:
        std::string varName;
    };

    class NewThread : public Statement {
    public:
        explicit NewThread(std::vector<std::unique_ptr<Statement>> && body);
        void print(std::ostream & out) const override;
        void accept(Visitor & visitor) override;
    private:
        std::vector<std::unique_ptr<Statement>> body;
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
    // TODO Dump

    class Statement::Visitor  {
    public:
        virtual void visitAssignStrong(AssignStrong & assignStrong);
        virtual void visitAssignWeak(AssignWeak & assignWeak);
        virtual void visitNewThread(NewThread & newThread);
        virtual void visitSleep(Sleep & sleep);
        virtual void visitSleepr(Sleepr & sleepr);
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
