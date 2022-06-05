#include <cassert>
#include "ir.h"

void NewObject::print(std::ostream & out) const {
    out << "object";
}

Var::Var(std::string const & name) : name(name) {}

void Var::print(std::ostream & out) const {
    out << name;
}

SelectField::SelectField(std::unique_ptr<AssignableTo> && obj, std::string const & name)
        : obj(std::move(obj)), name(name) {}

void SelectField::print(std::ostream & out) const {
    obj->print(out);
    out << "." << name;
}

AssignStrong::AssignStrong(std::unique_ptr<AssignableTo> && to, std::unique_ptr<Expression> && from)
        : to(std::move(to)), from(std::move(from)) {}

void AssignStrong::print(std::ostream & out) const {
    to->print(out);
    out << " = ";
    from->print(out);
}

AssignWeak::AssignWeak(std::unique_ptr<AssignableTo> && to, std::unique_ptr<Expression> && from)
        : to(std::move(to)), from(std::move(from)) {}

void AssignWeak::print(std::ostream & out) const {
    to->print(out);
    out << " ~= ";
    from->print(out);
}

EndOfLife::EndOfLife(std::string const & var)
        : varName(var) {}

void EndOfLife::print(std::ostream & out) const {
    out << "EndOfLife(" << varName << ")";
}

void EndOfLife::accept(Statement::Visitor & visitor) {
    visitor.visitEndOfLife(*this);
}

void Statement::Visitor::visitAssignStrong(AssignStrong & assignStrong) {
    visitStatement(assignStrong);
}

void Statement::Visitor::visitAssignWeak(AssignWeak & assignWeak) {
    visitStatement(assignWeak);
}

void Statement::Visitor::visitEndOfLife(EndOfLife & endOfLife) {
    visitStatement(endOfLife);
}

void Statement::Visitor::visitStatement(Statement & stat) {
    assert(false); // TODO fatal
}

void Expression::Visitor::visitNewObject(NewObject & newObject) {
    visitExpression(newObject);
}

void Expression::Visitor::visitVar(Var & var) {
    visitExpression(var);
}

void Expression::Visitor::visitSelectField(SelectField & selectField) {
    visitExpression(selectField);
}

void Expression::Visitor::visitExpression(Expression & expr) {
    assert(false); // TODO fatal
}

// TODO reorder?
void NewObject::accept(Expression::Visitor & visitor) {
    visitor.visitNewObject(*this);
}

void Var::accept(Expression::Visitor & visitor) {
    visitor.visitVar(*this);
}

void SelectField::accept(Expression::Visitor & visitor) {
    visitor.visitSelectField(*this);
}

void AssignStrong::accept(Statement::Visitor & visitor) {
    visitor.visitAssignStrong(*this);
}

void AssignWeak::accept(Statement::Visitor & visitor) {
    visitor.visitAssignWeak(*this);
}
