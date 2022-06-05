#include <cassert>
#include "interpreter.h"
#include "ast.h"

Interpreter::Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog) : prog(prog) {}

void Interpreter::interpret() {
    for (auto & elem: prog) {
        std::clog << "Interpreting ";
        elem->print(std::clog);
        std::clog << std::endl;

        elem->accept(*this);
    }
}

void Interpreter::visitAssignStrong(ast::AssignStrong & assignStrong) {
    auto evaluator = Evaluator(*this);
    auto obj = evaluator.evaluate(*assignStrong.from);

    auto scopeResolver = TargetResolver(*this);
    auto var = scopeResolver.resolveVar(*assignStrong.to);
    var->putStrong(obj); // FIXME obj got dead by this point
}

void Interpreter::visitAssignWeak(ast::AssignWeak & assignWeak) { // TODO generalize
    auto evaluator = Evaluator(*this);
    auto obj = evaluator.evaluate(*assignWeak.from);

    auto scopeResolver = TargetResolver(*this);
    auto var = scopeResolver.resolveVar(*assignWeak.to);
    var->putWeak(obj);
}

void Interpreter::visitEndOfLife(ast::EndOfLife & endOfLife) {
    // TODO assert no uses after
    globals.erase(endOfLife.varName);
}

Interpreter::~Interpreter() {
    std::clog << "=====================================================================" << std::endl;
    std::clog << "The interpreter is gone" << std::endl;
    std::clog << "=====================================================================" << std::endl;
}

TargetResolver::TargetResolver(Interpreter & interp)
        : interp(interp)
        , targetVar(nullptr) {}

Var * TargetResolver::resolveVar(ast::AssignableTo & assignableTo) {
    assert(targetVar == nullptr);
    assignableTo.accept(*this);
    assert(targetVar != nullptr);
    return targetVar;
}

void TargetResolver::visitVar(ast::Var & astVar) {
    auto & refToVar = interp.globals.getOrCreate(astVar.name);
    targetVar = refToVar.getRaw(); // has to be alive
}

void TargetResolver::visitSelectField(ast::SelectField & selectField) {
    auto evaluator = Evaluator(interp);
    auto obj = evaluator.evaluate(*selectField.obj);
    targetVar = &obj->fields.getOrCreate(selectField.name);
}

Evaluator::Evaluator(Interpreter & interp)
        : interp(interp)
        , result(nullptr) {}

Object * Evaluator::evaluate(ast::Expression & expr) {
    assert(result == nullptr);
    expr.accept(*this);
    assert(result != nullptr);
    return result;
}

void Evaluator::visitNewObject(ast::NewObject & newObject) {
    result = new Object();
}

void Evaluator::visitVar(ast::Var & astVar) {
    auto scopeResolver = TargetResolver(interp);
    auto var = scopeResolver.resolveVar(astVar);
    result = var->get();
}

void Evaluator::visitSelectField(ast::SelectField & selectField) {
    auto scopeResolver = TargetResolver(interp);
    auto var = scopeResolver.resolveVar(selectField);
    result = var->get();
}
