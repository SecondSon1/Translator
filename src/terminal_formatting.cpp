#include "terminal_formatting.hpp"

#include <sstream>

// Formatting
std::wostream &format::reset(std::wostream &out) {
  out << "\033[0m";
  return out;
}

std::wostream &format::bright(std::wostream &out) {
  out << "\033[1m";
  return out;
}

std::wostream &format::dim(std::wostream &out) {
  out << "\033[2m";
  return out;
}

std::wostream &format::italic(std::wostream &out) {
  out << "\033[3m";
  return out;
}

std::wostream &format::underline(std::wostream &out) {
  out << "\033[4m";
  return out;
}

std::wostream &format::blink(std::wostream &out) {
  out << "\033[5m";
  return out;
}

std::wostream &format::invert(std::wostream &out) {
  out << "\033[7m";
  return out;
}

std::wostream &format::hidden(std::wostream &out) {
  out << "\033[8m";
  return out;
}

std::wostream &format::cross(std::wostream &out) {
  out << "\033[9m";
  return out;
}

std::wostream &format::doubleunderline(std::wostream &out) {
  out << "\033[21m";
  return out;
}


// Foreground colors
std::wostream &color::black(std::wostream &out) {
  out << "\033[30m";
  return out;
}

std::wostream &color::red(std::wostream &out) {
  out << "\033[31m";
  return out;
}

std::wostream &color::green(std::wostream &out) {
  out << "\033[32m";
  return out;
}

std::wostream &color::yellow(std::wostream &out) {
  out << "\033[33m";
  return out;
}

std::wostream &color::blue(std::wostream &out) {
  out << "\033[34m";
  return out;
}

std::wostream &color::purple(std::wostream &out) {
  out << "\033[35m";
  return out;
}

std::wostream &color::cyan(std::wostream &out) {
  out << "\033[36m";
  return out;
}

std::wostream &color::white(std::wostream &out) {
  out << "\033[37m";
  return out;
}

std::wostream &color::light::black(std::wostream &out) {
  out << "\033[90m";
  return out;
}

std::wostream &color::light::red(std::wostream &out) {
  out << "\033[91m";
  return out;
}

std::wostream &color::light::green(std::wostream &out) {
  out << "\033[92m";
  return out;
}

std::wostream &color::light::yellow(std::wostream &out) {
  out << "\033[93m";
  return out;
}

std::wostream &color::light::blue(std::wostream &out) {
  out << "\033[94m";
  return out;
}

std::wostream &color::light::purple(std::wostream &out) {
  out << "\033[95m";
  return out;
}

std::wostream &color::light::cyan(std::wostream &out) {
  out << "\033[96m";
  return out;
}

std::wostream &color::light::white(std::wostream &out) {
  out << "\033[97m";
  return out;
}


// Background colors
std::wostream &color::background::black(std::wostream &out) {
  out << "\033[40m";
  return out;
}

std::wostream &color::background::red(std::wostream &out) {
  out << "\033[41m";
  return out;
}

std::wostream &color::background::green(std::wostream &out) {
  out << "\033[42m";
  return out;
}

std::wostream &color::background::yellow(std::wostream &out) {
  out << "\033[43m";
  return out;
}

std::wostream &color::background::blue(std::wostream &out) {
  out << "\033[44m";
  return out;
}

std::wostream &color::background::purple(std::wostream &out) {
  out << "\033[45m";
  return out;
}

std::wostream &color::background::cyan(std::wostream &out) {
  out << "\033[46m";
  return out;
}

std::wostream &color::background::white(std::wostream &out) {
  out << "\033[47m";
  return out;
}

std::wostream &color::background::light::black(std::wostream &out) {
  out << "\033[100m";
  return out;
}

std::wostream &color::background::light::red(std::wostream &out) {
  out << "\033[101m";
  return out;
}

std::wostream &color::background::light::green(std::wostream &out) {
  out << "\033[102m";
  return out;
}

std::wostream &color::background::light::yellow(std::wostream &out) {
  out << "\033[103m";
  return out;
}

std::wostream &color::background::light::blue(std::wostream &out) {
  out << "\033[104m";
  return out;
}

std::wostream &color::background::light::purple(std::wostream &out) {
  out << "\033[105m";
  return out;
}

std::wostream &color::background::light::cyan(std::wostream &out) {
  out << "\033[106m";
  return out;
}

std::wostream &color::background::light::white(std::wostream &out) {
  out << "\033[107m";
  return out;
}
