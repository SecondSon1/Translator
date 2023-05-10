#include "io.hpp"

#include <fstream>
#include <string>


bool readFile(const std::string & fileName, std::wstring & text) {
  std::wifstream file(fileName);
  if (!file.is_open()) return false;

  std::wstring line;
  while (std::getline(file, line)) {
    text += line;
    text.push_back(L'\n');
  }
  file.close();

  return true;
}
