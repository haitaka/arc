#include <cassert>
#include <sstream>
#include "parser.h"
#include "../ast.h"

SyntaxError::SyntaxError(uint line, uint pos, std::string const & msg) {
    std::stringstream buf;
    buf << "Syntax error at " << line << ":" << pos << " " << msg;
    descr = buf.str();
}

char const * SyntaxError::what() const noexcept {
    return descr.c_str();
}

Parser::Parser(char const * prog)
        : prog(prog)
        , lexer(prog)
        , nextToken() {
    consumeToken();
}

bool Parser::hasNext() const {
    return nextToken.kind != Token::Kind::End;
}

std::unique_ptr<ast::Statement> Parser::nextStatement() {
    assert(hasNext());
    assert(nextToken.kind != Token::Kind::Comment);
    return statement(true);
}

Token Parser::consumeToken(std::initializer_list<Token::Kind> kinds) {
    requireNext(kinds);
    return consumeToken();
}

void Parser::requireNext(std::initializer_list<Token::Kind> kinds) const {
    assert(kinds.size() > 0);
    bool validKind = false;
    for (auto expectedKind : kinds) {
        if (nextToken.kind == expectedKind) {
            validKind = true;
        }
    }
    if (!validKind) {
        // report an error
        throw makeExpectedFoundError(kinds);
    }
}

SyntaxError Parser::makeExpectedFoundError(std::initializer_list<Token::Kind> kinds) const {
    auto [line, posInLine] = lineAndPos();
    std::stringstream msg;
    msg << "expected ";
    bool first = true;
    for (auto expectedKind : kinds) {
        if (!first) {
            msg << " or ";
        }
        msg << expectedKind;
        first = false;
    }
    msg << ", found " << nextToken.range;
    return {line, posInLine, msg.str()};
}

std::pair<uint, uint> Parser::lineAndPos() const {
    uint posInLine = 1;
    uint line = 1;
    uint pos = nextToken.range.data() - prog;
    for (uint i = 0; i < pos; ++i) {
        if (prog[i] == '\n') {
            line += 1;
            posInLine = 1;
        } else {
            posInLine += 1;
        }
    }
    return std::make_pair(line, posInLine);
}

Token Parser::consumeToken() {
    auto current = nextToken;
    nextToken = lexer.next();
    while (nextToken.kind == Token::Kind::Comment && nextToken.kind != Token::Kind::End) {
        nextToken = lexer.next();
    }
    return current;
}

std::unique_ptr<ast::Statement> Parser::statement(bool topLevel) {
    if (nextToken.kind == Token::Kind::Thread) {
        if (topLevel) {
            return newThread();
        } else {
            throw makeExpectedFoundError({Token::Kind::Sleep, Token::Kind::Sleepr, Token::Kind::Dump, Token::Kind::Ident});
        }
    }
    if (nextToken.kind == Token::Kind::Sleep) {
        consumeToken({Token::Kind::Sleep});
        return std::make_unique<ast::Sleep>();
    }
    if (nextToken.kind == Token::Kind::Sleepr) {
        consumeToken({Token::Kind::Sleepr});
        return std::make_unique<ast::Sleepr>();
    }
    if (nextToken.kind == Token::Kind::Dump) {
        consumeToken({Token::Kind::Dump});
        return std::make_unique<ast::Dump>(expression());
    }
    // assignments
    auto to = assignableTo();
    auto assignOp = consumeToken({Token::Kind::Eq, Token::Kind::TildEq});
    auto from = expression();

    if (assignOp.kind == Token::Kind::Eq) {
        return std::make_unique<ast::Assign>(std::move(to), std::move(from), false);
    } else if (assignOp.kind == Token::Kind::TildEq) {
        return std::make_unique<ast::Assign>(std::move(to), std::move(from), true);
    } else {
        assert(false);
    }
}

std::unique_ptr<ast::NewThread> Parser::newThread() {
    consumeToken({Token::Kind::Thread});
    consumeToken({Token::Kind::LBrace});

    auto body = std::vector<std::unique_ptr<ast::Statement>>();
    while (nextToken.kind != Token::Kind::RBrace) {
        auto stat = statement(false);
        body.push_back(std::move(stat));
    }
    consumeToken({Token::Kind::RBrace});

    return std::make_unique<ast::NewThread>(std::move(body));
}

std::unique_ptr<ast::AssignableTo> Parser::assignableTo() {
    auto var = ident();
    std::unique_ptr<ast::AssignableTo> assignableTo = std::make_unique<ast::Var>(var);
    while (nextToken.kind == Token::Kind::Dot) {
        consumeToken({Token::Kind::Dot});
        auto field = ident();
        assignableTo = std::make_unique<ast::SelectField>(std::move(assignableTo), field);
    }

    return assignableTo;
}

std::unique_ptr<ast::Expression> Parser::expression() {
    if (nextToken.kind == Token::Kind::Object) {
        consumeToken({Token::Kind::Object});
        std::string name; // a string for debug purposes
        if (nextToken.kind == Token::Kind::LParenth) {
            consumeToken({Token::Kind::LParenth});
            name = std::string(consumeToken({Token::Kind::Ident}).range);
            consumeToken({Token::Kind::RParenth});
        }
        return std::make_unique<ast::NewObject>(name);
    }
    return assignableTo();
}

std::string Parser::ident() {
    return std::string(consumeToken({Token::Kind::Ident}).range);
}
