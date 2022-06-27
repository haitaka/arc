#include "preparation.h"
#include "ast.h"

#include <iostream>
#include <unordered_set>
#include <forward_list>
#include <cassert>

class UsageFinder : public ast::Statement::Visitor, public ast::Expression::Visitor {
public:
    std::unordered_set<std::string> usedVars;
private:
    void visitAssign(ast::Assign & assign) override {
        assign.to->accept(*this);
        assign.from->accept(*this);
    }

    void visitNewThread(ast::NewThread & newThread) override {
        for (auto & stat : newThread.body) {
            stat->accept(*this);
        }
    }

    void visitSleep(ast::Sleep & sleep) override {}

    void visitSleepr(ast::Sleepr & sleepr) override {}

    void visitDump(ast::Dump & dump) override {
        dump.expr->accept(*this);
    }

    void visitNewObject(ast::NewObject & newObject) override {}

    void visitVar(ast::Var & var) override {
        usedVars.insert(var.name);
    }

    void visitSelectField(ast::SelectField & selectField) override {
        selectField.obj->accept(*this);
    }
};

class VarsUsedInThreadCollector : public ast::Statement::Visitor {
public:
    explicit VarsUsedInThreadCollector(std::unordered_set<std::string> & usedVars) : usedVars(usedVars) {}

    void visitNewThread(ast::NewThread & newThread) override {
        newThread.usedVars = usedVars;
    }

    void visitStatement(ast::Statement & stat) override {}

private:
    std::unordered_set<std::string> & usedVars;
};

std::unordered_set<std::string> preprocess(std::vector<std::unique_ptr<ast::Statement>> & prog) {
    std::forward_list<std::unique_ptr<ast::Statement>> processedProg;
    std::unordered_set<std::string> aliveAndUndeclaredVars;
    for (auto iter = prog.rbegin(); iter != prog.rend(); iter++) {
        auto & stat = *iter;
        auto usageFinder = UsageFinder();
        stat->accept(usageFinder);
        auto threadPatcher = VarsUsedInThreadCollector(usageFinder.usedVars);
        stat->accept(threadPatcher);
        for (auto const & usedVar: usageFinder.usedVars) {
            if (aliveAndUndeclaredVars.count(usedVar) == 0) {
                processedProg.push_front(std::make_unique<ast::EndOfLife>(usedVar));
                aliveAndUndeclaredVars.insert(usedVar);
            }
        }
        processedProg.push_front(std::move(stat));
    }
    prog.clear();
    for (auto & stat: processedProg) {
        prog.push_back(std::move(stat));
    }
    return aliveAndUndeclaredVars;
}
