#include "lexer.h"

#include <cctype>
#include <cassert>
#include <string>
#include <string_view>

std::ostream & operator <<(std::ostream & out, Token::Kind const & kind) {
    switch (kind) {
        case Token::Kind::Dot: out << "`.`"; break;
        case Token::Kind::Eq: out << "`=`"; break;
        case Token::Kind::TildEq: out << "`~=`"; break;
        case Token::Kind::Object: out << "`object`"; break;
        case Token::Kind::Thread: out << "`thread`"; break;
        case Token::Kind::LBrace: out << "`{`"; break;
        case Token::Kind::RBrace: out << "`}`"; break;
        case Token::Kind::LParenth: out << "`(`"; break;
        case Token::Kind::RParenth: out << "`)`"; break;
        case Token::Kind::Sleep: out << "`sleep`"; break;
        case Token::Kind::Sleepr: out << "`sleepr`"; break;
        case Token::Kind::Dump: out << "`dump`"; break;
        case Token::Kind::Ident: out << "Identifier"; break;
        case Token::Kind::Comment: out << "Comment"; break;
        case Token::Kind::Invalid: out << "Invalid token"; break;
        case Token::Kind::End: out << "End"; break;
    }
    return out;
}

Token::Token()
        : kind(Kind::End)
        , range() {}

Token::Token(Token::Kind kind, char const * begin, size_t len)
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
    } else if (peeked == '(') {
        return atom(Token::Kind::LParenth, "(");
    } else if (peeked == ')') {
        return atom(Token::Kind::RParenth, ")");
    } else if (peeked == '/') {
        return comment();
    } else if (isIdentChar(peeked)) {
        return word();
    } else {
        // invalid
        char const * begin = remainder;
        while (std::isspace(peek())) {
            get();
        }
        char const * end = remainder;
        return {Token::Kind::Invalid, begin, end};
    }
}

char Lexer::peek() const {
    return *remainder;
}

char Lexer::get() {
    return *(remainder++);
}

bool Lexer::isIdentChar(char peeked) {
    // we are OK with idents starting with a digit
    return isalnum(peeked) || peeked == '_';
}

Token Lexer::atom(Token::Kind kind, std::string const & exact) {
    char const * begin = remainder;
    assert(std::string_view(begin, exact.length()) == exact);
    remainder += exact.length();
    return {kind, begin, exact.length()};
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
    return {Token::Kind::Comment, begin, end};
}

Token Lexer::word() {
    char const * begin = remainder;
    assert(isIdentChar(peek()));
    while (isIdentChar(peek())) {
        get();
    }
    char const * end = remainder;
    auto word = std::string_view(begin, std::distance(begin, end));
    if (word == "object") {
        return {Token::Kind::Object, begin, end};
    } else if (word == "thread") {
        return {Token::Kind::Thread, begin, end};
    } else if (word == "sleep") {
        return {Token::Kind::Sleep, begin, end};
    } else if (word == "sleepr") {
        return {Token::Kind::Sleepr, begin, end};
    } else if (word == "dump") {
        return {Token::Kind::Dump, begin, end};
    }
    return {Token::Kind::Ident, begin, end};
}
