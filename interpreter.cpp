#include <cassert>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include "interpreter.h"
#include "ast.h"
#include "logger.h"

Interpreter::Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog, std::unordered_set<std::string> const & globalNames)
    : prog(prog)
    , globals(globalNames) {}

Interpreter::Interpreter(std::vector<std::unique_ptr<ast::Statement>> const & prog, Globals && globals)
    : prog(prog)
    , globals(std::move(globals)) {}

void Interpreter::interpret() {
    for (auto & elem: prog) {
        interpret(elem);
    }

    for (auto & thread : subThreads) {
        thread.join();
    }
}

void Interpreter::interpret(std::unique_ptr<ast::Statement> const & stat) {
    std::stringstream buf;
    stat->print(buf);
    buf << std::endl;
    log(buf);

    stat->accept(*this);
}

void Interpreter::visitAssign(ast::Assign & assign) {
    auto evaluator = Evaluator(*this, *assign.from);
    auto & obj = evaluator.eval();

    RefToObj ref;
    if (assign.isWeak) {
        ref = std::move(RefToObj::makeWeak(obj));
    } else {
        ref = std::move(RefToObj::makeStrong(obj));
    }
    auto resolver = AssignableResolver(*this, *assign.to);
    resolver.scope().put(resolver.name(), std::move(ref));
}

void Interpreter::visitNewThread(ast::NewThread & astNewThread) {
    log("Starting new thread");
    auto usedGlobals = globals.makeSubsetInitIfNeeded(astNewThread.usedVars);
    auto threadInterpreter = new Interpreter(astNewThread.body, std::move(usedGlobals));

    auto thread = std::thread([this, & astNewThread, threadInterpreter](){
        threadInterpreter->interpret();
        delete threadInterpreter;
        log("Finished a thread");
    });
    subThreads.push_back(std::move(thread));
}

void Interpreter::visitSleep(ast::Sleep & sleep) {
    using namespace std::chrono_literals;
    // TODO maybe its not the interpreter who sets the delay size
    std::this_thread::sleep_for(100ms);
}

void Interpreter::visitSleepr(ast::Sleepr & sleepr) {
    using namespace std::chrono_literals;
    auto duration = 10 + std::rand() / ((RAND_MAX + 1u) / 90);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
}

std::ostream & operator<<(std::ostream & out, RefToObj const & ref) {
    auto rawPtr = ref.getRaw();
    if (ref.isWeak()) {
        Object * objPtr;
        try {
            objPtr = ref.get();
        } catch (WeakRef::InvalidAccess & ex) {
            objPtr = nullptr;
        }
        out << "weak(" << rawPtr << " -> " << objPtr << ")";
    } else {
        out << "strong(" << rawPtr << ")";
    }
    return out;
}

void Interpreter::visitDump(ast::Dump & dump) {
    std::stringstream buf;
    {
        auto evaluator = Evaluator(*this, *dump.expr);
        auto & ref = evaluator.eval();
        dump.print(buf);
        buf << ": ";
        buf << ref << ", ";

        if (ref.isWeak()) {
            buf << "weak ";
        } else {
            buf << "obj ";
        }
        // actual counter is +1 greater than an external observer would expect due to `obj` being a reference itself
        buf << "refCounter = " << ref.getRaw()->getRefCounter() - 1 << ", ";

        Object * obj = nullptr;
        try {
            obj = ref.get();
        } catch (WeakRef::InvalidAccess & ex) {
            buf << "obj collected" << std::endl;
        }

        if (obj != nullptr) {
            if (ref.isWeak()) {
                buf << "obj refCounter = " << ref->getRefCounter() << ", ";
            }

            auto fields = ref->getFields().getMap();
            buf << "fields = {";
            bool first = true;
            for (auto &[name, refInField]: fields) {
                if (!first) {
                    buf << ", ";
                }
                buf << name << ": " << refInField;
                first = false;
            }
            buf << "}" << std::endl;
        }
    }
    std::cout << buf.str();
}

void Interpreter::visitEndOfLife(ast::EndOfLife & endOfLife) {
    globals.erase(endOfLife.varName);
}

Interpreter::~Interpreter() {
    log("=====================================================================");
    log("The interpreter is gone");
    log("=====================================================================");
}

AssignableResolver::AssignableResolver(Interpreter & interp, ast::AssignableTo & assignableTo)
        : interp(interp)
        , containingObj()
        , varName() {
    assignableTo.accept(*this);
}

Scope & AssignableResolver::scope() const {
    if (containingObj.isEmpty()) {
        return interp.globals;
    } else {
        return containingObj->getFields();
    }
}

std::string AssignableResolver::name() const {
    assert(!varName.empty());
    return varName;
}

void AssignableResolver::visitVar(ast::Var & astVar) {
    assert(containingObj.isEmpty());
    varName = astVar.name;
}

void AssignableResolver::visitSelectField(ast::SelectField & selectField) {
    auto evaluator = Evaluator(interp, *selectField.obj);
    containingObj = std::move(evaluator.eval());
    varName = selectField.name;
}

Evaluator::Evaluator(Interpreter & interp, ast::Expression & expr)
        : interp(interp)
        , result() {
    expr.accept(*this);
}

RefToObj & Evaluator::eval() {
    assert(!result.isEmpty());
    return result;
}

void Evaluator::visitNewObject(ast::NewObject & newObject) {
    result = std::move(RefToObj::newStrong(new Object(newObject.name)));
}

void Evaluator::visitVar(ast::Var & var) {
    visitAssignableTo(var);
}

void Evaluator::visitSelectField(ast::SelectField & selectField) {
    visitAssignableTo(selectField);
}

void Evaluator::visitAssignableTo(ast::AssignableTo & assignableTo) {
    auto resolver = AssignableResolver(interp, assignableTo);
    result = std::move(resolver.scope().get(resolver.name()));
}
