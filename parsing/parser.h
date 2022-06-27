#pragma once

#include <memory>
#include <unordered_map>

#include "lexer.h"
#include "../ast.h"

class SyntaxError : public std::exception {
public:
    SyntaxError(uint line, uint pos, std::string const & msg);
    [[nodiscard]] char const * what() const noexcept override;
private:
    std::string descr;
};

class Parser {
public:
    explicit Parser(char const * prog);
    std::unique_ptr<ast::Statement> nextStatement();
    [[nodiscard]] bool hasNext() const;
private:
    Token consumeToken(std::initializer_list<Token::Kind> kinds);
    void requireNext(std::initializer_list<Token::Kind> kinds) const;
    SyntaxError makeExpectedFoundError(std::initializer_list<Token::Kind> kinds) const;
    [[nodiscard]] std::pair<uint, uint> lineAndPos() const;
    Token consumeToken();
    std::unique_ptr<ast::Statement> statement(bool topLevel);
    std::unique_ptr<ast::NewThread> newThread();
    std::unique_ptr<ast::AssignableTo> assignableTo();
    std::unique_ptr<ast::Expression> expression();
    std::string ident();

    char const * prog;
    Lexer lexer;
    Token nextToken;
};