#include "run.h"

#include "parsing/parser.h"
#include "preparation.h"
#include "interpreter.h"

void run(std::string const & prog) {
    Parser parser(prog.c_str());
    std::vector<std::unique_ptr<ast::Statement>> statements;
    while (parser.hasNext()) {
        statements.push_back(parser.nextStatement());
    }

    auto globalNames = preprocess(statements);

    Interpreter interp(statements, globalNames);
    interp.interpret();
}
