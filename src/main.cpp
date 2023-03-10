#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include "lexical_analyzer.hpp"
#include "logging.hpp"
#include "syntax_analysis.hpp"
#include "exceptions.hpp"
#include "terminal_colors.hpp"

void PrintHelp() {
	// TODO
	std::wcout << "Example help" << std::endl;
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
	std::wcout << color::bright << color::red << "Cannot open file " << color::reset << argv[1] << std::endl;
	return 1;
  }

  std::wstring code;
  std::wstring line;
  while (std::getline(codeFile, line)) {
    code += line;
    code.push_back(L'\n');
  }
  codeFile.close();

  auto lexemes = PerformLexicalAnalysis(code);

  // For testing, you can comment this
  log::warning(code, lexemes, 11);

  try {
    PerformSyntaxAnalysis(lexemes);
  } catch (const SyntaxAnalysisError & e) {
    log::error(code, lexemes, e);
    std::wcout << "Terminated, " << color::bright << color::red << '1' << color::reset << " error was found" << std::endl;
    return 2;
  }
  std::wcout << color::green << color::bright << '0' << color::reset << " errors were found, compiling..." << std::endl;
  return 0;
}
