#include "lexer.h"

#include <cctype>
#include <cassert>
#include <string>
#include <string_view>

std::ostream & operator <<(std::ostream & out, Token::Kind const & kind) {
    switch (kind) {
        case Token::Kind::Dot: out << "Dot"; break;
        case Token::Kind::Eq: out << "Eq"; break;
        case Token::Kind::TildEq: out << "TildEq"; break;
        case Token::Kind::Object: out << "Object"; break;
        case Token::Kind::Ident: out << "Ident"; break;
        case Token::Kind::Comment: out << "Comment"; break;
        case Token::Kind::End: out << "End"; break;
    }
    return out;
}

Token::Token()
        : kind(Kind::End)
        , range() {}

Token::Token(Token::Kind kind, char const * begin, uint len)
        : kind(kind)
        , range(begin, len) {}

Token::Token(Token::Kind kind, char const * begin, char const * end)
        : Token(kind, begin, std::distance(begin, end)) {}

Lexer::Lexer(char const * prog) : remainder(prog) {}

Token Lexer::next() {
    while (isspace(peek())) {
        get();
    }

    auto peeked = peek();
    if (peeked == '\0') {
        return atom(Token::Kind::End, "");
    } else if (peeked == '.') {
        return atom(Token::Kind::Dot, ".");
    } else if (peeked == '=') {
        return atom(Token::Kind::Eq, "=");
    } else if (peeked == '~') {
        return atom(Token::Kind::TildEq, "~=");
    } else if (peeked == '{') {
        return atom(Token::Kind::LBrace, "{");
    } else if (peeked == '}') {
        return atom(Token::Kind::RBrace, "}");
    } else if (peeked == '/') {
        return comment();
    } else if (isalpha(peeked)) {
        return word();
    } else {
        assert(false); // TODO fatal
    }
}

char Lexer::peek() const {
    return *remainder;
}

char Lexer::get() {
    return *(remainder++);
}

Token Lexer::atom(Token::Kind kind, std::string const & exact) {
    char const * begin = remainder;
    assert(std::string_view(begin, exact.length()) == exact);
    remainder += exact.length();
    return Token(kind, begin, exact.length());
}

Token Lexer::comment() {
    assert(peek() == '/');
    char const * begin = remainder;
    get();
    assert(peek() == '/');
    while (peek() != '\0' && peek() != '\n') {
        get();
    }
    char const * end = remainder;
    return Token(Token::Kind::Comment, begin, end);
}

Token Lexer::word() {
    char const * begin = remainder;
    assert(isalpha(peek()));
    while (isalnum(peek())) {
        get();
    }
    char const * end = remainder;
    auto word = std::string_view(begin, std::distance(begin, end));
    if (word == "object") {
        return Token(Token::Kind::Object, begin, end);
    } else if (word == "thread") {
        return Token(Token::Kind::Thread, begin, end);
    } else if (word == "sleep") {
        return Token(Token::Kind::Sleep, begin, end);
    } else if (word == "sleepr") {
        return Token(Token::Kind::Sleepr, begin, end);
    } else if (word == "dump") {
        return Token(Token::Kind::Dump, begin, end);
    }
    return Token(Token::Kind::Ident, begin, end);
}
