#pragma once

#include <fstream>
#include <string>

#include "run.hpp"

bool readFile(const std::string & fileName, std::wstring & text);

bool saveProgram(const std::string & fileName, const RPN & rpn);

bool readProgram(const std::string & fileName, RPN & rpn);
