#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

#include "run.h"

std::string getFileContent(std::string const & path) {
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

void printUsageAndDie() {
    std::cerr << "Usage: arc <script.arc>" << std::endl;
    exit(1);
}

int main(int argc, char ** argv) {
    if (argc != 2) {
        printUsageAndDie();
    }
    auto filename = argv[1];
    auto prog = getFileContent(filename);

    run(prog);

    return 0;
}
