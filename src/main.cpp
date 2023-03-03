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

  try {
    PerformSyntaxAnalysis(z);
    std::wcout << L"Syntax Analysis: OK" << std::endl;
  } catch (SyntaxAnalysisError & e) {
    std::wcout << L"Syntax Analysis: Error" << std::endl;
    size_t index = z[e.GetIndex()].GetIndex();
    if (e.GetIndex() == z.size()) {
      --index;
      std::wcout << L"Expected: " << ToString(e.GetExpected()) << L", got: EOF" << std::endl;
    } else {
      std::wcout << L"Expected: " << ToString(e.GetExpected()) << L", got: " << ToString(z[e.GetIndex()].GetType()) << std::endl;
    }
    size_t cnt = 0;
    std::wstring prev;
    for (size_t i = 0; cnt <= index; ++i) {
      line.clear();
      for (; i < code.size() && code[i] != L'\n'; ++i) {
        line.push_back(code[i]);
      }
      if (cnt + line.size() > index) {
        std::wcout << std::endl;
        if (!prev.empty())
          std::wcout << prev << std::endl;
        std::wcout << line << std::endl;
        index -= cnt;
        break;
      } else cnt += line.size() + 1;
      prev = line;
    }
    while (index-- > 0) std::wcout << L' ';
    std::wcout << "^" << std::endl;

  }

  return 0;
}
