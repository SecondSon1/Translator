#include <iostream>
#include <cstdlib>
#include <string>

#include "lexeme.hpp"
#include "lexical_analyzer.hpp"
#include "syntax_analysis.hpp"
#include "exceptions.hpp"

int32_t main() {

  std::wstring code;
  std::wstring line;
  while (std::getline(std::wcin, line)) {
    code += line;
    code.push_back(L'\n');
  }

  auto z = PerformLexicalAnalysis(code);

  int index = -11;
  try {
    PerformSyntaxAnalysis(z);
  } catch (SyntaxAnalysisError & e) {
    index = e.GetIndex();
    std::wcout << L"Error at syntax analysis at lexeme " << e.GetIndex() << std::endl;
    std::wcout << L"Expected: " << ToString(e.GetExpected()) << L", got "
                << ToString(z[e.GetIndex()].GetType()) << std::endl;
  }

  int i = 0;
  for (auto lexeme : z) {
    if (i == index)
      std::wcout << "> ";
    if (index - 10 <= i && i <= index + 10)
      std::wcout << i << ": " << lexeme << std::endl;
    ++i;
  }

  return 0;
}
