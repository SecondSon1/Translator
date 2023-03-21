#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include "lexical_analyzer.hpp"
#include "logging.hpp"
#include "syntax_analyzer.hpp"
#include "exceptions.hpp"
#include "terminal_formatting.hpp"

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
      std::wcout << format::bright << color::red << "Cannot open file " << format::reset << argv[1] << std::endl;
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

  try {
    PerformSyntaxAnalysis(lexemes);
  } catch (const UnexpectedLexeme & e) {
    log::error(code, lexemes, e);
    std::wcout << "Terminated, " << format::bright << color::red << '1' << format::reset << " error was found" << std::endl;
    return 2;
  }
  std::wcout << color::green << format::bright << '0' << format::reset << " errors were found, compiling..." << std::endl;
  return 0;
}
