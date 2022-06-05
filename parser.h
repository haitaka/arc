#pragma once

#include <memory>

#include "lexer.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(char const * prog);
    std::unique_ptr<ast::Statement> nextStatement();
    bool hasNext();
private:
    void consumeToken();
    std::unique_ptr<ast::Statement> statement();
    std::unique_ptr<ast::AssignableTo> assignableTo();
    std::unique_ptr<ast::Expression> expression();

    Lexer lexer;
    Token nextToken;
};