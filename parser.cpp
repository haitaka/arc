#include <cassert>
#include "parser.h"
#include "ast.h"

// TODO handle EOS

Parser::Parser(char const * prog)
        : lexer(prog)
        , nextToken() {
    consumeToken();
}

bool Parser::hasNext() {
    return nextToken.kind != Token::Kind::End;
}

std::unique_ptr<ast::Statement> Parser::nextStatement() {
    assert(hasNext());
    assert(nextToken.kind != Token::Kind::Comment);
    return statement();
}

void Parser::consumeToken() {
    nextToken = lexer.next();
    while (nextToken.kind == Token::Kind::Comment && nextToken.kind != Token::Kind::End) {
        nextToken = lexer.next();
    }
}

std::unique_ptr<ast::Statement> Parser::statement() {
    // TODO others
    if (nextToken.kind == Token::Kind::Thread) {
        return newThread();
    }
    if (nextToken.kind == Token::Kind::Sleep) {
        consumeToken();
        return std::make_unique<ast::Sleep>();
    }
    if (nextToken.kind == Token::Kind::Sleepr) {
        consumeToken();
        return std::make_unique<ast::Sleepr>();
    }
    // assignments
    auto to = assignableTo();
    auto assignOp = nextToken;
    consumeToken();
    auto from = expression();

    if (assignOp.kind == Token::Kind::Eq) {
        return std::make_unique<ast::AssignStrong>(std::move(to), std::move(from));
    } else if (assignOp.kind == Token::Kind::TildEq) {
        return std::make_unique<ast::AssignWeak>(std::move(to), std::move(from));
    } else {
        assert(false); // TODO
        return nullptr;
    }
}

std::unique_ptr<ast::NewThread> Parser::newThread() {
    assert(nextToken.kind == Token::Kind::Thread);
    consumeToken();
    assert(nextToken.kind == Token::Kind::LBrace);
    consumeToken();

    auto body = std::vector<std::unique_ptr<ast::Statement>>();
    while (nextToken.kind != Token::Kind::RBrace) {
        auto stat = statement();
        // TODO require that stat is not newThread
        body.push_back(std::move(stat));
    }
    consumeToken();

    return std::make_unique<ast::NewThread>(std::move(body));
}

std::unique_ptr<ast::AssignableTo> Parser::assignableTo() {
    assert(nextToken.kind == Token::Kind::Ident);
    std::unique_ptr<ast::AssignableTo> assignableTo = std::make_unique<ast::Var>(std::string(nextToken.range));
    consumeToken();
    while (nextToken.kind == Token::Kind::Dot) {
        consumeToken();
        assert(nextToken.kind == Token::Kind::Ident);
        assignableTo = std::make_unique<ast::SelectField>(std::move(assignableTo), std::string(nextToken.range)); // FIXME why all these std::move?
        consumeToken();
    }

    return assignableTo;
}

std::unique_ptr<ast::Expression> Parser::expression() {
    if (nextToken.kind == Token::Kind::Object) {
        consumeToken();
        return std::make_unique<ast::NewObject>();
    }
    return assignableTo();
}
