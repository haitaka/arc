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
        LParenth,
        RParenth,
        Sleep,
        Sleepr,
        Dump,
        Ident,
        Comment,
        Invalid,
        End,
    };

    Token();
    Token(Kind kind, char const * begin, size_t len);
    Token(Kind kind, char const * begin, char const * end);

    Kind kind;
    std::string_view range;
};

std::ostream & operator <<(std::ostream & out, Token::Kind const & kind);

class Lexer {
public:
    explicit Lexer(const char * prog);
    Token next();
private:
    [[nodiscard]] char peek() const;
    char get();

    Token atom(Token::Kind, std::string const & exact);
    Token comment();
    Token word();

    const char * remainder;

    [[nodiscard]] static bool isIdentChar(char peeked) ;
};