#pragma once

#include <thread>
#include <iostream>
#include <sstream>

// Ideally, the logger should be a special implementation of std::ostream, to avoid all this buffer stuff.

bool const ENABLED = false;

void log(std::string const & msg);

void log(std::stringstream const & buf);
