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
	std::wcout << L"Example help" << std::endl;
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
	std::wcout << color::red << "Cannot open file " << color::reset << argv[1] << std::endl;
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
  } catch (SyntaxAnalysisError & e) {
    size_t index = z[e.GetIndex()].GetIndex();

    // Getting lexeme position in file
    size_t lineIndex = 1, columnIndex = 0, lineStartIndex;
    for (size_t i = 0; i < index; ++i, ++columnIndex) {
        if (code[i] == '\n') {
            columnIndex = 0;
            ++lineIndex;
            lineStartIndex = i + 1;
        }
    }

    // Printing error info
    std::wcout << color::bright << argv[1] << ':' << lineIndex << ':' << columnIndex << ": ";
    std::wcout << color::red << "error: " << color::reset << color::bright << "unexpected lexeme" << color::reset;

    if (e.GetIndex() == z.size()) {
        std::wcout << color::bright << " (";
        std::wcout << "expected: " << color::cyan << ToString(e.GetExpected()) << color::reset;
        std::wcout << color::bright << ", got: " << color::red << "EOF" << color::reset;
        std::wcout << color::bright << ')' << color::reset << std::endl;
        --index;
    } else {
        std::wcout << color::bright << " (";
        std::wcout << "expected: " << color::cyan << ToString(e.GetExpected()) << color::reset;
        std::wcout << color::bright << ", got: " << color::red << ToString(z[e.GetIndex()].GetType()) << color::reset;
        std::wcout << color::bright << ')' << color::reset << std::endl;
    }

    // Printing line with error
    for (size_t i = lineStartIndex; i < code.size() && code[i] != '\n'; ++i) {
        if (i == index) std::wcout << color::red << color::underline;
        else if (i == index + z[e.GetIndex()].GetValue().size()) std::wcout << color::reset;
        std::wcout << code[i];
    }
    std::wcout << std::endl << std::endl;

    std::wcout << color::red << color::bright << '1' << color::reset << " error was found" << std::endl;
    return 2;
  }

  std::wcout << color::green << color::bright << '0' << color::reset << " errors were found" << std::endl;
  return 0;
}
