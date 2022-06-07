#include <cassert>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "interpreter.h"
#include "ast.h"

Interpreter::Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog) : prog(prog) {}

void Interpreter::interpret() {
    for (auto & elem: prog) {
        interpret(elem);
    }

    for (auto & thread : threads) {
        thread.join();
    }
}

void Interpreter::interpret(std::unique_ptr<ast::Statement> const & stat) {
    std::clog << "Interpreting ";
    stat->print(std::clog);
    std::clog << std::endl;

    stat->accept(*this);
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

void Interpreter::visitNewThread(ast::NewThread & newThread) { // TODO generalize
    std::clog << "Starting new thread" << std::endl;
    auto thread = std::thread([this, & newThread](){
        for (auto & stat : newThread.body) {
            interpret(stat);
        }
        std::clog << "Finished a thread" << std::endl;
    });
    threads.push_back(std::move(thread));
}

void Interpreter::visitSleep(ast::Sleep & sleep) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
}

void Interpreter::visitSleepr(ast::Sleepr & sleepr) {
    using namespace std::chrono_literals;
    auto duration = 10 + std::rand() / ((RAND_MAX + 1u) / 90);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
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
    // FIXME
    if (refToVar.getRaw() == nullptr) {
        refToVar = std::move(StrongRefToVar(new Collectible<Var>()));
    }
    targetVar = refToVar.getRaw(); // has to be alive
}

void TargetResolver::visitSelectField(ast::SelectField & selectField) {
    auto evaluator = Evaluator(interp);
    auto obj = evaluator.evaluate(*selectField.obj);
    targetVar = &obj->content.fields.getOrCreate(selectField.name);
}

Evaluator::Evaluator(Interpreter & interp)
        : interp(interp)
        , result(nullptr) {}

Collectible<Object> * Evaluator::evaluate(ast::Expression & expr) {
    assert(result == nullptr);
    expr.accept(*this);
    assert(result != nullptr);
    return result;
}

void Evaluator::visitNewObject(ast::NewObject & newObject) {
    result = new Collectible<Object>();
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
