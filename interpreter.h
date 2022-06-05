#pragma once

#include <vector>
#include <unordered_map>

#include "ir.h"
#include "object.h"
#include "reference.h"

class Reference;
class Object;

// TODO create class?
typedef std::unordered_map<std::string, Reference> Scope;

class Interpreter : public Statement::Visitor {
public:
    explicit Interpreter(std::vector<std::unique_ptr<Statement>> const & prog);
    ~Interpreter();
    void interpret();
public:
    std::vector<std::unique_ptr<Statement>> const & prog;
    Scope globals;
private:

    void visitAssignStrong(AssignStrong & assignStrong) override;
    void visitAssignWeak(AssignWeak & assignWeak) override;

    void visitEndOfLife(EndOfLife & endOfLife) override;

    class TargetResolver : public Expression::Visitor {
    public:
        explicit TargetResolver(Interpreter & interp);
        std::pair<Scope *, std::string *> resolveScopeAndName(AssignableTo & assignableTo);
    private:
        Interpreter & interp;
        Scope * scope;
        std::string * name;

        void visitVar(Var & var) override;
        void visitSelectField(SelectField & selectField) override;
    };

    class Evaluator : public Expression::Visitor {
    public:
        explicit Evaluator(Interpreter & interp);
        Object * evaluate(Expression & expr);
    private:
        Interpreter & interp;
        Object * result;

        void visitNewObject(NewObject & newObject) override;
        void visitVar(Var & var) override;
        void visitSelectField(SelectField & selectField) override;
    };
};

