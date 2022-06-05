#pragma once

#include <memory>

#include "lexer.h"
#include "ir.h"

class Parser {
public:
    explicit Parser(char const * prog);
    std::unique_ptr<Statement> nextStatement();
    bool hasNext();
private:
    void consumeToken();
    std::unique_ptr<Statement> statement();
    std::unique_ptr<AssignableTo> assignableTo();
    std::unique_ptr<Expression> expression();

    Lexer lexer;
    Token nextToken;
};