#include "preparation.h"
#include "ast.h"

#include <iostream>
#include <unordered_set>
#include <cassert>

class UsageFinder : public ast::Statement::Visitor, public ast::Expression::Visitor {
public:
    std::unordered_set<std::string> usedVars;
private:
    void visitAssignStrong(ast::AssignStrong & assignStrong) override {
        assignStrong.to->accept(*this);
        assignStrong.from->accept(*this);
    }

    void visitAssignWeak(ast::AssignWeak & assignWeak) override {
        assignWeak.to->accept(*this);
        assignWeak.from->accept(*this);
    }

    void visitNewObject(ast::NewObject & newObject) override {
        // nop
    }

    void visitVar(ast::Var & var) override {
        usedVars.insert(var.name);
    }

    void visitSelectField(ast::SelectField & selectField) override {
        selectField.obj->accept(*this);
    }
};

// TODO generalize?
void prepare(std::vector<std::unique_ptr<ast::Statement>> & prog) {
    std::unordered_set<std::string> aliveVars; // FIXME alive or not yet declared
    for (uint i = prog.size(); i > 0; --i) {
        uint j = i - 1;
        assert(j < prog.size());
        auto & stat = prog[j];
        auto usageFinder = UsageFinder();
        stat->accept(usageFinder);
        for (auto const & usedVar: usageFinder.usedVars) {
            if (aliveVars.count(usedVar) == 0) {
                prog.insert(prog.begin() + j + 1, std::make_unique<ast::EndOfLife>(usedVar));
                aliveVars.insert(usedVar);
            }
        }
    }
}
