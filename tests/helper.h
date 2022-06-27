#pragma once

#include <string>

std::string const & prog(std::string const & line);

std::string prog(std::initializer_list<std::string> chunks);

std::string repeat(uint times, std::string const & counter, std::initializer_list<std::string> chunks);
