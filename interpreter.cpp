#include <cassert>
#include "interpreter.h"

Interpreter::Interpreter(std::vector<std::unique_ptr<Statement>> const & prog) : prog(prog) {}

void Interpreter::interpret() {
    for (auto & elem: prog) {
        std::clog << "Interpreting ";
        elem->print(std::clog);
        std::clog << std::endl;

        elem->accept(*this);
    }
}

void Interpreter::visitAssignStrong(AssignStrong & assignStrong) {
    auto evaluator = Evaluator(*this);
    auto obj = evaluator.evaluate(*assignStrong.from);

    auto scopeResolver = TargetResolver(*this);
    auto scopeAndName = scopeResolver.resolveScopeAndName(*assignStrong.to);
    auto & scope = *scopeAndName.first;
    auto & name = *scopeAndName.second;
    // FIXME what address change on about rehash?
    scope.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(obj, true)); // FIXME ?
}

void Interpreter::visitAssignWeak(AssignWeak & assignWeak) { // TODO generalize
    auto evaluator = Evaluator(*this);
    auto obj = evaluator.evaluate(*assignWeak.from);

    auto scopeResolver = TargetResolver(*this);
    auto scopeAndName = scopeResolver.resolveScopeAndName(*assignWeak.to);
    auto & scope = *scopeAndName.first;
    auto & name = *scopeAndName.second;
    scope.emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(obj, false)); // FIXME ?
}

void Interpreter::visitEndOfLife(EndOfLife & endOfLife) {
    // TODO assert no uses after
    globals.erase(endOfLife.varName);
}

Interpreter::~Interpreter() {
    std::clog << "=====================================================================" << std::endl;
    std::clog << "The interpreter is gone" << std::endl;
    std::clog << "=====================================================================" << std::endl;
}

Interpreter::TargetResolver::TargetResolver(Interpreter & interp)
        : interp(interp)
        , scope(nullptr)
        , name(nullptr) {}

std::pair<Scope *, std::string *> Interpreter::TargetResolver::resolveScopeAndName(AssignableTo & assignableTo) {
    assert(scope == nullptr);
    assert(name == nullptr);
    assignableTo.accept(*this);
    assert(scope != nullptr);
    assert(name != nullptr);
    return std::make_pair(scope, name);
}

void Interpreter::TargetResolver::visitVar(Var & var) {
    scope = &interp.globals;
    name = &var.name;
}

void Interpreter::TargetResolver::visitSelectField(SelectField & selectField) {
    auto evaluator = Evaluator(interp);
    auto obj = evaluator.evaluate(*selectField.obj);
    scope = &obj->fields;
    name = &selectField.name;
}

Interpreter::Evaluator::Evaluator(Interpreter & interp)
        : interp(interp)
        , result(nullptr) {}

Object * Interpreter::Evaluator::evaluate(Expression & expr) {
    assert(result == nullptr);
    expr.accept(*this);
    assert(result != nullptr);
    return result;
}

void Interpreter::Evaluator::visitNewObject(NewObject & newObject) {
    result = new Object();
}

void Interpreter::Evaluator::visitVar(Var & var) {
    auto scopeResolver = TargetResolver(interp);
    auto scope = scopeResolver.resolveScopeAndName(var).first;
    result = scope->at(var.name).get();
}

void Interpreter::Evaluator::visitSelectField(SelectField & selectField) {
    auto scopeResolver = TargetResolver(interp);
    auto scope = scopeResolver.resolveScopeAndName(selectField).first;
    result = scope->at(selectField.name).get();
}
