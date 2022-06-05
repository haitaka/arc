#include "preparation.h"

#include <iostream>
#include <unordered_set>
#include <cassert>

class UsageFinder : public Statement::Visitor, public Expression::Visitor {
public:
    std::unordered_set<std::string> usedVars;
private:
    void visitAssignStrong(AssignStrong & assignStrong) override {
        assignStrong.to->accept(*this);
        assignStrong.from->accept(*this);
    }

    void visitAssignWeak(AssignWeak & assignWeak) override {
        assignWeak.to->accept(*this);
        assignWeak.from->accept(*this);
    }

    void visitNewObject(NewObject & newObject) override {
        // nop
    }

    void visitVar(Var & var) override {
        usedVars.insert(var.name);
    }

    void visitSelectField(SelectField & selectField) override {
        selectField.obj->accept(*this);
    }
};

// TODO generalize?
void prepare(std::vector<std::unique_ptr<Statement>> & prog) {
    std::unordered_set<std::string> aliveVars; // FIXME alive or not yet declared
    for (uint i = prog.size(); i > 0; --i) {
        uint j = i - 1;
        assert(j < prog.size());
        auto & stat = prog[j];
        auto usageFinder = UsageFinder();
        stat->accept(usageFinder);
        for (auto const & usedVar: usageFinder.usedVars) {
            if (aliveVars.count(usedVar) == 0) {
                prog.insert(prog.begin() + j + 1, std::make_unique<EndOfLife>(usedVar));
                aliveVars.insert(usedVar);
            }
        }
    }
}
