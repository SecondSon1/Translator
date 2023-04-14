#include "syntax_analyzer.hpp"
#include "exceptions.hpp"
#include "lexeme.hpp"

#define DEBUG_ACTIVE 0

#if defined(DEBUG_ACTIVE) && DEBUG_ACTIVE
#include <iostream>
void debug_out() { std::wcerr << std::endl; }
template <typename Head, typename... Tail> void debug_out(Head H, Tail... T) { std::wcerr << L" " << H; debug_out(T...);}
#define debug(...) std::wcerr << "[" << __LINE__ << "]:", debug_out(__VA_ARGS__)
#else
#define debug(...) 42
#endif

std::vector<Lexeme> _lexemes;
size_t _lexeme_index;
bool eof;
Lexeme lexeme;

void GetNext() {
  _lexeme_index++;
  if (_lexeme_index == _lexemes.size()) {
      eof = true;
      lexeme = Lexeme(LexemeType::kNull, L"", lexeme.GetIndex() + lexeme.GetValue().size());
  }
  else
      lexeme = _lexemes[_lexeme_index];
}

void Program();

void PerformSyntaxAnalysis(const std::vector<Lexeme> & code) {
  if (code.empty()) return;
  _lexemes = code;
  _lexeme_index = 0;
  lexeme = code[0];
  eof = false;
  Program();
}

void Expect(LexemeType type) {
  if (eof || lexeme.GetType() != type)
    throw UnexpectedLexeme(lexeme, type);
}

void Expect(LexemeType type, LexemeType others...) {
  if (lexeme.GetType() == type) return;
  Expect(others);
}

void Expect(LexemeType type, const std::wstring & value) {
  if (eof || lexeme.GetType() != type || lexeme.GetValue() != value)
    throw UnexpectedLexeme(lexeme, type);
}

bool IsLexeme(LexemeType type) {
  return !eof && lexeme.GetType() == type;
}
bool IsLexeme(LexemeType type, const std::wstring & value) {
  return !eof && lexeme.GetType() == type && lexeme.GetValue() == value;
}
bool IsLexeme(const std::wstring & value) {
  return !eof && lexeme.GetValue() == value;
}

void Action();
void Block();
void Keyword();
void Expression();
void Struct();
void VariableIdentifier();
void Definition();
void VariableParameter();
void ParameterList();
void LambdaFunction();
void Function();
void Type();
void TypeNoConst();
void If();
void Elif();
void Else();
void For();
void Foreach();
void While();
void DoWhile();
void Try();
void Throw();
void Continue();
void Break();
void Return();
void FunctionCall();
void Priority1();
void Priority2();
void Priority3();
void Priority4();
void Priority5();
void Priority6();
void Priority7();
void Priority8();
void Priority9();
void Priority10();
void Priority11();
void Priority12();
void Priority13();
void Priority14();
void Priority15();
void Priority16();

void Program() {
  debug("Program");
  while (!eof) {
    Action();
  }
}

void Action() {
  debug("Action");
  if (IsLexeme(LexemeType::kReserved) || IsLexeme(LexemeType::kVariableType))
    Keyword();
  else if (IsLexeme(LexemeType::kPunctuation, L"{"))
    Block();
  else {
    Expression();
    Expect(LexemeType::kPunctuation, L";");
    GetNext();
  }
  debug("Exited action");
}

void Block() {
  debug("Block");
  if (!IsLexeme(LexemeType::kPunctuation, L"{")) {
    Action();
    return;
  }
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
      Action();
  }
  Expect(LexemeType::kPunctuation, L"}");
  GetNext();
}

void Keyword() {
  debug("Keyword");
  Expect(LexemeType::kReserved, LexemeType::kVariableType);
  if (IsLexeme(L"for"))
    For();
  else if (IsLexeme(L"while"))
    While();
  else if (IsLexeme(L"do"))
    DoWhile();
  else if (IsLexeme(L"foreach"))
    Foreach();
  else if (IsLexeme(L"if") || IsLexeme(L"elif") || IsLexeme(L"else"))
    If();
  else if (IsLexeme(L"return"))
    Return();
  else if (IsLexeme(L"break"))
    Break();
  else if (IsLexeme(L"continue"))
    Continue();
  else if (IsLexeme(L"struct"))
      Struct();
  else if (IsLexeme(L"try"))
      Try();
  else if (IsLexeme(L"throw"))
      Throw();
  else
    Definition();
}

void Expression() {
  debug("Expression");
  Priority1();
}

void EpsExpression() {
  debug("EpsExpression");
  if (IsLexeme(LexemeType::kPunctuation, L";"))
    return;
  Expression();
}

void Struct() {
  debug("Struct");
  Expect(LexemeType::kReserved, L"struct");
  GetNext();
  Expect(LexemeType::kIdentifier);
  GetNext();
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
    Definition();
  }
  Expect(LexemeType::kPunctuation, L"}");
  GetNext();
}

void VariableIdentifier() {
  debug("Variable Identifier");
  Expect(LexemeType::kIdentifier);
  GetNext();
  while (IsLexeme(LexemeType::kBracket, L"[")) {
    GetNext();
    Expression();
    Expect(LexemeType::kBracket, L"]");
    GetNext();
  }
}

void Definition() {
  debug("Definition");
  Type();
  Expect(LexemeType::kIdentifier);
  GetNext();
  if (IsLexeme(LexemeType::kParenthesis)) {
    Function();
    return;
  }
  while (IsLexeme(LexemeType::kBracket, L"[")) {
    GetNext();
    Expression();
    Expect(LexemeType::kBracket, L"]");
    GetNext();
  }
  if (IsLexeme(LexemeType::kOperator, L"=")) {
    GetNext();
    Expression();
  }
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    Expect(LexemeType::kIdentifier);
    GetNext();
    while (IsLexeme(LexemeType::kBracket, L"[")) {
      GetNext();
      Expression();
      Expect(LexemeType::kBracket, L"]");
      GetNext();
    }
    if (IsLexeme(LexemeType::kOperator, L"=")) {
      GetNext();
      Expression();
    }
  }
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void VariableParameter() {
  Type();
  VariableIdentifier();
}

void ParameterList() {
  if (IsLexeme(LexemeType::kParenthesis, L")")) return;

  bool startedDefault = false;

  VariableParameter();
  if (IsLexeme(LexemeType::kOperator, L"=")) {
    startedDefault = true;
    GetNext();
    Expression();
  }

  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    VariableParameter();
    if (IsLexeme(LexemeType::kOperator, L"=")) {
      startedDefault = true;
    }
    if (startedDefault) {
      Expect(LexemeType::kOperator, L"=");
      GetNext();
      Expression();
    }
  }
}

void LambdaFunction(); // TODO: think...

void Function() {
  debug("Function");
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  ParameterList();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    Block();
  debug("Exited Function");
}

void Type() {
  debug("Type");
  if (IsLexeme(LexemeType::kReserved, L"const")) {
    GetNext();
  }
  TypeNoConst();
  if (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
  }
}

void TypeNoConst() {
  if (!IsLexeme(LexemeType::kVariableType) && !IsLexeme(LexemeType::kIdentifier))
    Expect(LexemeType::kVariableType);
  GetNext();
}

void If() {
  debug("If");
  Expect(LexemeType::kReserved, L"if");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  debug("lol");
  Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
  Elif();
  if (IsLexeme(LexemeType::kReserved, L"else"))
    Else();
}

void Elif() {
  while (IsLexeme(LexemeType::kReserved, L"elif")) {
    GetNext();
    Expect(LexemeType::kParenthesis, L"(");
    GetNext();
    Expression();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    Block();
  }
}

void Else() {
  Expect(LexemeType::kReserved, L"else");
  GetNext();
  Block();
}

void For() {
  debug("For");
  Expect(LexemeType::kReserved, L"for");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    Definition();
  else
    GetNext();
  EpsExpression();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  EpsExpression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
  debug("Exited For");
}

void Foreach() {
  Expect(LexemeType::kReserved, L"foreach");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  VariableParameter();
  Expect(LexemeType::kReserved, L"of");
  GetNext();
  Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
}

void While() {
  Expect(LexemeType::kReserved, L"while");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
}

void DoWhile() {
  Expect(LexemeType::kReserved, L"do");
  GetNext();
  Block();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  Expect(LexemeType::kReserved, L"while");
  GetNext();
  Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Try() {
  Expect(LexemeType::kReserved, L"try");
  GetNext();
  Block();
  Expect(LexemeType::kReserved, L"catch");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  VariableParameter();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
  while (IsLexeme(LexemeType::kReserved, L"catch")) {
    GetNext();
    Expect(LexemeType::kParenthesis, L"(");
    GetNext();
    VariableParameter();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    Block();
  }
}

void Throw() {
  Expect(LexemeType::kReserved, L"throw");
  GetNext();
  Expression();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Continue() {
  Expect(LexemeType::kReserved, L"continue");
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Break() {
  Expect(LexemeType::kReserved, L"break");
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Return() {
  Expect(LexemeType::kReserved, L"return");
  GetNext();
  Expression();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void FunctionCall() {
  debug("Function Call");
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  if (!IsLexeme(LexemeType::kParenthesis, L")")) {
    Expression();
    while (IsLexeme(LexemeType::kPunctuation, L",")) {
      GetNext();
      Expression();
    }
  }
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
}

const size_t signs1_sz = 12;
const std::wstring signs1[signs1_sz] = {
  L"=", L"<<=", L">>=", L"+=", L"-=", L"*=", L"**=", L"/=",
  L"^=", L"&=", L"|=", L"%="
};

void Priority1() {
  Priority2();

  while (true) {
    bool found = false;
    for (size_t sign = 0; sign < signs1_sz; ++sign) {
      if (IsLexeme(signs1[sign])) {
        found = true;
        break;
      }
    }
    if (!found) break;
    GetNext();
    Priority2();
  }
}

void Priority2() {
  Priority3();
  while (IsLexeme(LexemeType::kOperator, L"&&")) {
    GetNext();
    Priority3();
  }
}

void Priority3() {
  Priority4();
  while (IsLexeme(LexemeType::kOperator, L"||")) {
    GetNext();
    Priority4();
  }
}

void Priority4() {
  Priority5();
  while (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
    Priority5();
  }
}

void Priority5() {
  Priority6();
  while (IsLexeme(LexemeType::kOperator, L"|")) {
    GetNext();
    Priority6();
  }
}

void Priority6() {
  Priority7();
  while (IsLexeme(LexemeType::kOperator, L"^")) {
    GetNext();
    Priority7();
  }
}

void Priority7() {
  Priority8();
  while (IsLexeme(LexemeType::kOperator, L"==")
    || IsLexeme(LexemeType::kOperator, L"!=")) {
    GetNext();
    Priority8();
  }
}

void Priority8() {
  Priority9();
  while (IsLexeme(LexemeType::kOperator, L"<")
    || IsLexeme(LexemeType::kOperator, L"<=")
    || IsLexeme(LexemeType::kOperator, L">")
    || IsLexeme(LexemeType::kOperator, L">=")) {
    GetNext();
    Priority9();
  }
}

void Priority9() {
  Priority10();
  while (IsLexeme(LexemeType::kOperator, L"<<")
    || IsLexeme(LexemeType::kOperator, L">>")) {
    GetNext();
    Priority10();
  }
}

void Priority10() {
  Priority11();
  while (IsLexeme(LexemeType::kOperator, L"+")
    || IsLexeme(LexemeType::kOperator, L"-")) {
    GetNext();
    Priority11();
  }
}

void Priority11() {
  Priority12();
  while (IsLexeme(LexemeType::kOperator, L"*")
    || IsLexeme(LexemeType::kOperator, L"/")
    || IsLexeme(LexemeType::kOperator, L"%")) {
    GetNext();
    Priority12();
  }
}

void Priority12() {
  Priority13();
  while (IsLexeme(LexemeType::kOperator, L"**")) {
    GetNext();
    Priority13();
  }
}

void Priority13() {
  if (IsLexeme(LexemeType::kOperator, L"!")) {
    GetNext();
  }
  Priority14();
}

void Priority14() {
  if (IsLexeme(LexemeType::kOperator, L"+")
    || IsLexeme(LexemeType::kOperator, L"-")
    || IsLexeme(LexemeType::kOperator, L"++")
    || IsLexeme(LexemeType::kOperator, L"--")) {
    GetNext();
  }
  Priority15();
}


void Priority15() {
  Priority16();
  if (IsLexeme(LexemeType::kOperator, L"++")
    || IsLexeme(LexemeType::kOperator, L"--")) {
    GetNext();
    return;
  }
  while (true) {
    if (IsLexeme(LexemeType::kBracket, L"[")) {
      GetNext();
      Expression();
      Expect(LexemeType::kBracket, L"]");
      GetNext();
    } else if (IsLexeme(LexemeType::kParenthesis, L"(")) {
      FunctionCall();
    } else if (IsLexeme(LexemeType::kOperator, L".")) {
      GetNext();
      Priority16();
    } else break;
  }
}
void Priority16() {
  if (IsLexeme(LexemeType::kNumericLiteral)
    || IsLexeme(LexemeType::kIdentifier)
    || IsLexeme(LexemeType::kStringLiteral)
    || IsLexeme(LexemeType::kReserved, L"true") || IsLexeme(LexemeType::kReserved, L"false")) {
    GetNext();
  }
  else {
    Expect(LexemeType::kParenthesis, L"(");
    GetNext();
    Expression();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
  }
}
