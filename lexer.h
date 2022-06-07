#pragma once

#include <iostream>
#include <string>
#include <string_view>

class Token {
public:
    enum class Kind {
        Dot,
        Eq,
        TildEq,
        Object,
        Thread,
        LBrace,
        RBrace,
        Sleep,
        Sleepr,
        Dump,
        Ident,
        Comment,
        End,
    };

    Token();
    Token(Kind kind, char const * begin, uint len);
    Token(Kind kind, char const * begin, char const * end);

    Kind kind;
    std::string_view range;
};

std::ostream & operator <<(std::ostream & out, Token::Kind const & kind);

class Lexer {
public:
    Lexer(const char * prog);
    Token next();
private:
    char peek() const;
    char get();

    Token atom(Token::Kind, std::string const & exact);
    Token comment();
    Token word();

    const char * remainder;
};