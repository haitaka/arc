#include "helper.h"

#include <sstream>
#include <algorithm>

using namespace std::string_literals;

std::string const & prog(std::string const & line) {
    return line;
}

std::string prog(std::initializer_list<std::string> chunks) {
    std::stringstream buf;
    bool first = true;
    for (auto & chunk : chunks) {
        if (!first) {
            buf << std::endl;
        }
        buf << chunk << std::endl;
        first = false;
    }
    return buf.str();
}

void replaceAll(std::string & str, std::string_view what, std::string_view with) {
    std::string::size_type pos = 0;
    while (true) {
        pos = str.find(what.data(), pos, what.length());
        if (pos == std::string::npos) {
            return;
        }
        str.replace(pos, what.length(), with.data(), with.length());
        pos += with.length();
    }
}

std::string repeat(uint times, std::string const & counter, std::initializer_list<std::string> chunks) {
    std::stringstream buf;
    bool first = true;
    for (uint i = 0; i < times; ++i) {
        for (auto chunk : chunks) {
            if (!first) {
                buf << std::endl;
            }
            replaceAll(chunk, "$"s + counter, std::to_string(i));
            buf << chunk << std::endl;
            first = false;
        }
    }

    return buf.str();
}
