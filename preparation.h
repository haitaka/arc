#pragma once

#include <vector>

#include "ast.h"

void prepare(std::vector<std::unique_ptr<ast::Statement>> & prog);
