#include "syntax_analysis.hpp"

std::vector<Lexeme> _lexemes;
size_t _lexeme_index;
bool eof;
Lexeme lexeme;

void PerformSyntaxAnalysis(const std::vector<Lexeme> & code) {
  if (code.empty()) return;
  _lexemes = code;
  _lexeme_index = 0;
  eof = false;
}



void GetNext() {
  if (_lexeme_index + 1 == _lexemes.size()) {
    eof = true;
  } else {
    lexeme = _lexemes[_lexeme_index + 1];
  }
}
