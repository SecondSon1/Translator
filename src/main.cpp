#include <iostream>
#include <cstdlib>
#include <string>

#include "lexeme.hpp"
#include "lexical_analyzer.hpp"

int32_t main() {

  std::wstring code;
  std::wstring line;
  while (std::getline(std::wcin, line)) {
    code += line;
    code.push_back(L'\n');
  }

  std::wcout << L"Lexemes:" << std::endl;
  auto x = GetLexemeStrings();
  for (auto [k, v] : x) {
    std::wcout << ToString(k) << L":";
    for (auto y : v)
      std::wcout << L" " << y;
    std::wcout << std::endl;
  }
  std::wcout << std::endl;
  

  std::wcout << code << std::endl;
  std::wcout << "Lexical analysis:" << std::endl;
  auto z = PerformLexicalAnalysis(code);
  std::wcout << std::endl;
  for (auto lexeme : z) {
    std::wcout << lexeme << std::endl;
  }

  return 0;
}
