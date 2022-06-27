#pragma once

#include <list>

#include "ast.h"

std::unordered_set<std::string> preprocess(std::vector<std::unique_ptr<ast::Statement>> & prog);
