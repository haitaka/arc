#pragma once

#include <vector>

#include "scope.h"
#include "ast.h"
#include "mm.h"

class Interpreter : public ast::Statement::Visitor {
public:
    explicit Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog);
    ~Interpreter();
    void interpret();
public:
    std::vector<std::unique_ptr<ast::Statement>> const & prog;
    Scope<StrongRefToVar> globals;
private:

    void visitAssignStrong(ast::AssignStrong & assignStrong) override;
    void visitAssignWeak(ast::AssignWeak & assignWeak) override;

    void visitEndOfLife(ast::EndOfLife & endOfLife) override;
};

class TargetResolver : public ast::Expression::Visitor {
public:
    explicit TargetResolver(Interpreter & interp);
    Var * resolveVar(ast::AssignableTo & assignableTo);
private:
    Interpreter & interp;
    Var * targetVar;

    void visitVar(ast::Var & astVar) override;
    void visitSelectField(ast::SelectField & selectField) override;
};

class Evaluator : public ast::Expression::Visitor {
public:
    explicit Evaluator(Interpreter & interp);
    Collectible<Object> * evaluate(ast::Expression & expr);
private:
    Interpreter & interp;
    Collectible<Object> * result;

    void visitNewObject(ast::NewObject & newObject) override;
    void visitVar(ast::Var & astVar) override;
    void visitSelectField(ast::SelectField & selectField) override;
};
