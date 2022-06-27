#include <cassert>
#include <string>
#include <sstream>
#include <utility>
#include "ast.h"

ast::NewObject::NewObject(std::string name) : name(std::move(name)) {}

void ast::NewObject::print(std::ostream & out) const {
    out << "object";
    if (name.length() > 0) {
        out << "(" << name << ")";
    }
}

void ast::NewObject::accept(ast::Expression::Visitor & visitor) {
    visitor.visitNewObject(*this);
}

ast::Var::Var(std::string name) : name(std::move(name)) {}

void ast::Var::print(std::ostream & out) const {
    out << name;
}

void ast::Var::accept(ast::Expression::Visitor & visitor) {
    visitor.visitVar(*this);
}

ast::SelectField::SelectField(std::unique_ptr<ast::AssignableTo> && obj, std::string name)
        : obj(std::move(obj)), name(std::move(name)) {}

void ast::SelectField::print(std::ostream & out) const {
    obj->print(out);
    out << "." << name;
}

void ast::SelectField::accept(Expression::Visitor & visitor) {
    visitor.visitSelectField(*this);
}

ast::Assign::Assign(std::unique_ptr<AssignableTo> && to, std::unique_ptr<Expression> && from, bool isWeak)
        : isWeak(isWeak)
        , to(std::move(to))
        , from(std::move(from)) {}

void ast::Assign::print(std::ostream & out) const {
    to->print(out);
    if (isWeak) {
        out << " ~= ";
    } else {
        out << " = ";
    }
    from->print(out);
}

void ast::Assign::accept(ast::Statement::Visitor & visitor) {
    visitor.visitAssign(*this);
}

ast::EndOfLife::EndOfLife(std::string var) : varName(std::move(var)) {}

void ast::EndOfLife::print(std::ostream & out) const {
    out << "EndOfLife(" << varName << ")";
}

void ast::EndOfLife::accept(ast::Statement::Visitor & visitor) {
    visitor.visitEndOfLife(*this);
}

ast::NewThread::NewThread(std::vector<std::unique_ptr<Statement>> && body)
        : body(std::move(body)) {}

void ast::NewThread::print(std::ostream & out) const {
    out << "thread {" << std::endl;
    for (auto const & stat: body) {
        out << "    ";
        stat->print(out);
        out << std::endl;
    }
    out << "}";
}

void ast::NewThread::accept(ast::Statement::Visitor & visitor) {
    visitor.visitNewThread(*this);
}


ast::Dump::Dump(std::unique_ptr<Expression> expr) : expr(std::move(expr)) {}

void ast::Dump::print(std::ostream & out) const {
    out << "dump ";
    expr->print(out);
}

void ast::Dump::accept(ast::Statement::Visitor & visitor) {
    visitor.visitDump(*this);
}

void ast::Sleep::print(std::ostream & out) const {
    out << "sleep";
}

void ast::Sleep::accept(ast::Statement::Visitor & visitor) {
    visitor.visitSleep(*this);
}

void ast::Sleepr::print(std::ostream & out) const {
    out << "sleepr";
}

void ast::Sleepr::accept(ast::Statement::Visitor & visitor) {
    visitor.visitSleepr(*this);
}

void ast::Statement::Visitor::visitAssign(ast::Assign & assign) {
    visitStatement(assign);
}

void ast::Statement::Visitor::visitNewThread(ast::NewThread & newThread) {
    visitStatement(newThread);
}

void ast::Statement::Visitor::visitSleep(ast::Sleep & sleep) {
    visitStatement(sleep);
}

void ast::Statement::Visitor::visitSleepr(ast::Sleepr & sleepr) {
    visitStatement(sleepr);
}

void ast::Statement::Visitor::visitDump(ast::Dump & dump) {
    visitStatement(dump);
}

void ast::Statement::Visitor::visitEndOfLife(ast::EndOfLife & endOfLife) {
    visitStatement(endOfLife);
}

void ast::Statement::Visitor::visitStatement(ast::Statement & stat) {
    assert(false);
}

void ast::Expression::Visitor::visitNewObject(ast::NewObject & newObject) {
    visitExpression(newObject);
}

void ast::Expression::Visitor::visitVar(ast::Var & var) {
    visitExpression(var);
}

void ast::Expression::Visitor::visitSelectField(ast::SelectField & selectField) {
    visitExpression(selectField);
}

void ast::Expression::Visitor::visitExpression(ast::Expression & expr) {
    assert(false);
}
