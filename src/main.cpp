#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include "lexeme.hpp"
#include "lexical_analyzer.hpp"
#include "syntax_analysis.hpp"
#include "exceptions.hpp"
#include "terminal_colors.hpp"

void PrintHelp() {
	// TODO
	std::cout << "Example help" << std::endl;
	return;
}

int32_t main(const int argc, const char *argv[]) {
  // Maybe it's better to create ParceArgs function
  // Or to do it in another file
  // I don't wanna to think too much
  if (argc < 2) {
      PrintHelp();
      return 0;
  }

  std::wifstream codeFile;
  codeFile.open(argv[1]);
  if (!codeFile.is_open()) {
	std::cout << color::red << "Cannot open file " << color::reset << argv[1] << std::endl;
	return 1;
  }

  std::wstring code;
  std::wstring line;
  while (std::getline(codeFile, line)) {
    code += line;
    code.push_back(L'\n');
  }
  codeFile.close();

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
