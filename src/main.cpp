#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include "lexeme.hpp"
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
  // Maybe it's better to create ParseArgs function
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
  log::init(code);

  try {
    std::vector<Lexeme> lexemes = PerformLexicalAnalysis(code);
    for (Lexeme lexeme : lexemes)
      if (lexeme.GetType() == LexemeType::kUnknown)
        throw UnknownLexeme(lexeme.GetIndex(), lexeme.GetValue());
    PerformSyntaxAnalysis(lexemes);
  }
  catch (const TranslatorError & e) {
    log::error(e);
    std::wcout << "Terminated, " << format::bright << color::red << '1' << format::reset << " error(s) were found" << std::endl;
    std::wcout << format::bright << color::blue << log::getWarningsNum() << format::reset << " warning(s) were generated" << std::endl;
    return 2;
  }
  catch (...) {
    std::wcout << "Something went wrong. We are sorry about it";
    return 3;
  }
  std::wcout << format::bright << color::green << '0' << format::reset << " error(s) were found" << std::endl;
  std::wcout << format::bright << color::blue << log::getWarningsNum() << format::reset << " warning(s) were generated" << std::endl;

  return 0;
}
