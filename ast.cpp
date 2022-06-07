#include <cassert>
#include "ast.h"

void ast::NewObject::print(std::ostream & out) const {
    out << "object";
}

ast::Var::Var(std::string const & name) : name(name) {}

void ast::Var::print(std::ostream & out) const {
    out << name;
}

ast::SelectField::SelectField(std::unique_ptr<ast::AssignableTo> && obj, std::string const & name)
        : obj(std::move(obj)), name(name) {}

void ast::SelectField::print(std::ostream & out) const {
    obj->print(out);
    out << "." << name;
}

ast::AssignStrong::AssignStrong(std::unique_ptr<ast::AssignableTo> && to, std::unique_ptr<ast::Expression> && from)
        : to(std::move(to)), from(std::move(from)) {}

void ast::AssignStrong::print(std::ostream & out) const {
    to->print(out);
    out << " = ";
    from->print(out);
}

ast::AssignWeak::AssignWeak(std::unique_ptr<ast::AssignableTo> && to, std::unique_ptr<ast::Expression> && from)
        : to(std::move(to)), from(std::move(from)) {}

void ast::AssignWeak::print(std::ostream & out) const {
    to->print(out);
    out << " ~= ";
    from->print(out);
}

ast::EndOfLife::EndOfLife(std::string const & var)
        : varName(var) {}

void ast::EndOfLife::print(std::ostream & out) const {
    out << "EndOfLife(" << varName << ")";
}

void ast::EndOfLife::accept(ast::Statement::Visitor & visitor) {
    visitor.visitEndOfLife(*this);
}

ast::NewThread::NewThread(std::vector<std::unique_ptr<Statement>> && body)
        : body(std::move(body)) {}

void ast::NewThread::print(std::ostream & out) const {
    out << "newThread {" << std::endl;
    for (auto const & stat : body) {
        out << "    ";
        stat->print(out);
        out << std::endl;
    }
    out << "}";
}

void ast::NewThread::accept(ast::Statement::Visitor & visitor) {
    visitor.visitNewThread(*this);
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

void ast::Statement::Visitor::visitAssignStrong(ast::AssignStrong & assignStrong) {
    visitStatement(assignStrong);
}

void ast::Statement::Visitor::visitAssignWeak(ast::AssignWeak & assignWeak) {
    visitStatement(assignWeak);
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

void ast::Statement::Visitor::visitEndOfLife(ast::EndOfLife & endOfLife) {
    visitStatement(endOfLife);
}

void ast::Statement::Visitor::visitStatement(ast::Statement & stat) {
    assert(false); // TODO fatal
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
    assert(false); // TODO fatal
}

// TODO reorder?
void ast::NewObject::accept(ast::Expression::Visitor & visitor) {
    visitor.visitNewObject(*this);
}

void ast::Var::accept(ast::Expression::Visitor & visitor) {
    visitor.visitVar(*this);
}

void ast::SelectField::accept(Expression::Visitor & visitor) {
    visitor.visitSelectField(*this);
}

void ast::AssignStrong::accept(ast::Statement::Visitor & visitor) {
    visitor.visitAssignStrong(*this);
}

void ast::AssignWeak::accept(Statement::Visitor & visitor) {
    visitor.visitAssignWeak(*this);
}
