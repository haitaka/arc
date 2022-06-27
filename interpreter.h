#pragma once

#include <vector>
#include <thread>
#include <functional>

#include "ast.h"
#include "mm.h"

class Interpreter : public ast::Statement::Visitor {
public:
    Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog, std::unordered_set<std::string> const & globalNames);
    Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog, Globals && globals);
    ~Interpreter();
    void interpret();
    void interpret(std::unique_ptr<ast::Statement> const & stat);
public:
    std::vector<std::unique_ptr<ast::Statement>> const & prog;
    Globals globals;
    std::vector<std::thread> subThreads;
private:

    void visitAssign(ast::Assign & assign) override;
    void visitNewThread(ast::NewThread & astNewThread) override;
    void visitSleep(ast::Sleep & sleep) override;
    void visitSleepr(ast::Sleepr & sleepr) override;
    void visitDump(ast::Dump & dump) override;

    void visitEndOfLife(ast::EndOfLife & endOfLife) override;
};

class AssignableResolver : public ast::Expression::Visitor {
public:
    explicit AssignableResolver(Interpreter & interp, ast::AssignableTo & assignableTo);
    [[nodiscard]] Scope & scope() const;
    [[nodiscard]] std::string name() const;
private:
    Interpreter & interp;
    RefToObj containingObj; // if the scope is a field, anchors the containing object
    std::string varName;

    void visitVar(ast::Var & astVar) override;
    void visitSelectField(ast::SelectField & selectField) override;
};

class Evaluator : public ast::Expression::Visitor {
public:
    explicit Evaluator(Interpreter & interp, ast::Expression & expr);
    RefToObj & eval();
private:
    Interpreter & interp;
    RefToObj result;

    void visitNewObject(ast::NewObject & newObject) override;
    void visitVar(ast::Var & var) override;
    void visitSelectField(ast::SelectField & selectField) override;

    void visitAssignableTo(ast::AssignableTo & assignableTo);
};
