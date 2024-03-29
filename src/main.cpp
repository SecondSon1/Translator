#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>

#include "TID.hpp"
#include "generation.hpp"
#include "lexeme.hpp"
#include "lexical_analyzer.hpp"
#include "logging.hpp"
#include "syntax_analyzer.hpp"
#include "exceptions.hpp"
#include "terminal_formatting.hpp"

#include "run.hpp"

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

std::vector<std::string> split(std::string str, const std::string & delimiter) {
  std::vector<std::string> result;
  size_t pos = 0;
  std::string token;

  while ((pos = str.find(delimiter)) != std::string::npos) {
    token = str.substr(0, pos);
    result.push_back(token);
    str.erase(0, pos + delimiter.length());
  }
  return result;
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
  catch (const std::exception & e) {
    std::wcout << "Something went wrong. We are sorry about it" << std::endl << e.what() << std::endl;
    return 3;
  }
  catch (...) {
    std::wcout << "Something went wrong. We are sorry about it" << std::endl;
    return 4;
  }
  std::wcout << format::bright << color::green << '0' << format::reset << " error(s) were found" << std::endl;
  std::wcout << format::bright << color::blue << log::getWarningsNum() << format::reset << " warning(s) were generated" << std::endl;
  std::wcout << std::endl << "Generated RPN:" << std::endl;
  size_t ind = 0;
  for (auto x : rpn.GetNodes())
    std::wcout << ind++ << L": " << x->ToString() << std::endl;
  std::wcout << std::endl << "Executing:" << std::endl;
  int32_t ret_code = Execute(rpn);
  std::wcout << L"Return code: " << std::to_wstring(ret_code) << std::endl;

  return 0;
}