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

std::map<std::string, std::string> options = {
    {"disableWarnings", "false"},
    {"compileFile",     ""},
    {"outFile",         ""},
    {"runFile",         ""},
};

void ParseArgs(const int argc, const char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compile") == 0) {
      if (i + 1 < argc) {
        options["compileFile"] = argv[++i];
      }
    }
    else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--out") == 0) {
      if (i + 1 < argc) {
        options["outFile"] = argv[++i];
      }
    }
    else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--run") == 0) {
      if (i + 1 < argc) {
        options["runFile"] = argv[++i];
      }
    }
    else if (strcmp(argv[i], "--disableWarnings") == 0) {
      options["disableWarnings"] = "true";
    }
  }
}

void PrintHelp() {
	// TODO
	std::wcout << "Example help" << std::endl;
	return;
}

int32_t main(const int argc, const char *argv[]) {
  if (argc < 2) {
    PrintHelp();
    return 0;
  }
  ParseArgs(argc, argv);

  std::wifstream codeFile;
  codeFile.open(options["compileFile"]);
  if (!codeFile.is_open()) {
    std::wcout << format::bright << color::red << "Cannot open file " << format::reset;
    std::cout << options["compileFile"] << std::endl;
    return 1;
  }

  std::wstring code;
  std::wstring line;
  while (std::getline(codeFile, line)) {
    code += line;
    code.push_back(L'\n');
  }
  codeFile.close();
  log::init(code, options);

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
