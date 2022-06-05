#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <memory>
#include <sstream>

#include "parser.h"
#include "ir.h"
#include "interpreter.h"
#include "preparation.h"

// TODO noexcept
// TODO default initializers ({}, = nullptr)
// TODO C++17

std::string getFileContent(std::string const & path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

int main(int argc, char ** argv) {
    assert(argc == 2); // TODO
    auto filename = argv[1];
    auto prog = getFileContent(filename);

    Parser parser(prog.c_str());
    std::vector<std::unique_ptr<Statement>> statements;
    while (parser.hasNext()) {
        statements.push_back(parser.nextStatement());
    }

    for (auto & stat: statements) {
        stat->print(std::cout);
        std::cout << std::endl;
    }

    prepare(statements);

    Interpreter interp(statements);
    interp.interpret();

    return 0;
}
