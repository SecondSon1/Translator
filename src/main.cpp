#include <iostream>
#include <string>

#include "generation.hpp"
#include "io.hpp"
#include "lexeme.hpp"
#include "lexical_analyzer.hpp"
#include "logging.hpp"
#include "syntax_analyzer.hpp"
#include "exceptions.hpp"
#include "run.hpp"
#include "terminal_formatting.hpp"

std::map<std::string, std::string> options = {
    {"disableWarnings", "false"},
    {"compileFile",     ""},
    {"outFile",         "out.bbl"},
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
	std::wcout << "Usage: bblc [-c | --compile <path>] [-o | --out <path>] [-r | --run <path>] [--disableWarnings]" << std::endl << std::endl;
  std::wcout << format::bright << "-c | --compile <path>" << format::reset << "   Compiling file given in <path>" << std::endl;
  std::wcout << format::bright << "-o | --out <path>" << format::reset << "       Writes compiled file in <path>" << std::endl;
  std::wcout << format::bright << "-r | --run <path>" << format::reset << "       Running file given in <path>" << std::endl;
  std::wcout << format::bright << "--disableWarnings" << format::reset << "       Disables all the warning during compilation" << std::endl;
  std::wcout << std::endl;
}

#define RPN_EXECUTING_TESTING 0

int32_t main(const int argc, const char *argv[]) {

#if defined(RPN_EXECUTING_TESTING) && RPN_EXECUTING_TESTING
  // TEMP, testing running;
  {
    RPN rpn;
    rpn.PushNode(RPNOperand(13 + 8 + 4 + 4));
    rpn.PushNode(RPNOperand(0));
    rpn.PushNode(RPNOperator(RPNOperatorType::kPush));
    rpn.PushNode(RPNOperand(-1ull));
    rpn.PushNode(RPNOperator(RPNOperatorType::kSP));
    rpn.PushNode(RPNOperator(RPNOperatorType::kStore, PrimitiveVariableType::kUint64));

    Execute(rpn);

    return 0;
  }
#endif

  if (argc < 2) {
    PrintHelp();
    return 0;
  }
  ParseArgs(argc, argv);

  if (options["compileFile"].size() > 0) {
    // Compiling
    std::wstring code;
    if (!readFile(options["compileFile"], code)) {
      std::wcout << format::bright << color::red << "Cannot open file " << format::reset;
      std::cout << options["compileFile"] << std::endl;
      return 1;
    }
    log::init(code, options);

    RPN rpn;
    try {
      std::vector<Lexeme> lexemes = PerformLexicalAnalysis(code);
      for (Lexeme lexeme : lexemes)
        if (lexeme.GetType() == LexemeType::kUnknown)
          throw UnknownLexeme(lexeme.GetIndex(), lexeme.GetValue());
      rpn = PerformSyntaxAnalysis(lexemes);
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

#if defined(RPN_EXECUTING_TESTING) && RPN_EXECUTING_TESTING
    std::wcout << std::endl << "Generated RPN:" << std::endl;
    size_t ind = 0;
    for (auto x : rpn.GetNodes())
      std::wcout << ind++ << L": " << x->ToString() << std::endl;
#endif

    if (!saveProgram(options["outFile"], rpn)) {
      std::wcout << format::bright << color::red << "Cannot open file " << format::reset;
      std::cout << options["outFile"] << std::endl;
      return 4;
    }
    std::wcout << format::bright << color::blue << "Saved to " << format::reset;
    std::cout << options["outFile"] << std::endl;
  }

  if (options["runFile"].size() > 0) {
    // Execute file
    RPN rpn;
    if (!readProgram(options["runFile"], rpn)) {
      std::wcout << format::bright << color::red << "Cannot open file " << format::reset;
      std::cout << options["runFile"] << std::endl;
      return 5;
    }
    int32_t ret_code = Execute(rpn);
    std::wcout << L"Return code: " << std::to_wstring(ret_code) << std::endl;
  }

  return 0;
}
