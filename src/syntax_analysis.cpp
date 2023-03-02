#include "syntax_analysis.hpp"
#include "exceptions.hpp"
#include "lexeme.hpp"

std::vector<Lexeme> _lexemes;
size_t _lexeme_index;
bool eof;
Lexeme lexeme;

void GetNext() {
  if (_lexeme_index + 1 == _lexemes.size()) {
    eof = true;
  } else {
    lexeme = _lexemes[_lexeme_index + 1];
  }
}

void Program();

void PerformSyntaxAnalysis(const std::vector<Lexeme> & code) {
  if (code.empty()) return;
  _lexemes = code;
  _lexeme_index = 0;
  eof = false;
}

void Expect(LexemeType type) {
  if (lexeme.GetType() != type)
    throw SyntaxAnalysisError(_lexeme_index, type);
}

void ExpectOneOf(LexemeType type) {
  
}

void Expect(LexemeType type, const std::wstring & value) {
  if (lexeme.GetType() != type || lexeme.GetValue() != value)
    throw SyntaxAnalysisError(_lexeme_index, type);
}

bool IsLexeme(LexemeType type) {
  return lexeme.GetType() == type;
}
bool IsLexeme(LexemeType type, const std::wstring & value) {
  return lexeme.GetType() == type && lexeme.GetValue() == value;
}
bool IsLexeme(const std::wstring & value) {
  return lexeme.GetValue() == value;
}



void Action();
void Block();
void Keyword();
void Expression();
void Struct();
void VariableIdentifier();
void VariableIdentifierList();
void Definition();
void VariableParameter();
void VariableParameterList();
void DefaultParameter();
void DefaultParameterList();
void ParameterList();
void LambdaFunction();
void Function();
void Type();
void TypeNoConst();
void If();
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



void Identifiers() {
  Expect(LexemeType::kIdentifier);
  GetNext();
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    Expect(LexemeType::kIdentifier);
    GetNext();
  }
}

void Action() {
  if (IsLexeme(LexemeType::kReserved))
    Keyword();
  else if (IsLexeme(LexemeType::kPunctuation, L"{"))
    Block();
  else
    Expression();
}

void Block() {
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();
  Program();
  Expect(LexemeType::kPunctuation, L"}");
  GetNext();
}

void Keyword() {
  Expect(LexemeType::kReserved);
  if (IsLexeme(L"for"))
    For();
  else if (IsLexeme(L"while"))
    While();
  else if (IsLexeme(L"do"))
    DoWhile();
  else if (IsLexeme(L"foreach"))
    Foreach();
  else if (IsLexeme(L"if")) {
    If();
    Elif();
    Else();
  }
  else if (IsLexeme(L"return"))
    Return();
  else if (IsLexeme(L"break"))
    Break();
  else if (IsLexeme(L"continue"))
    Continue();
  else if (IsLexeme(L"struct"))
    Struct();
  else
    Definition();
}

void Expression() {
  Priority0();
}

void EpsExpression() {
  if (IsLexeme(LexemeType::kPunctuation, L";"))
    return;
  Expression();
}

void Struct() {
  Expect(LexemeType::kReserved, L"struct");
  GetNext();
  Expect(LexemeType::kIdentifier);
  GetNext();
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
    Definition();
    Expect(LexemeType::kPunctuation, L";");
    GetNext();
  }
  Expect(LexemeType::kPunctuation, L"}");
  GetNext();
}

void VariableIdentifier() {
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
  Expect(LexemeType::kReserved);
}

void VariableParameter() {
  Type();
  VariableIdentifier();
}

void VariableParameterList() {
  VariableParameter();
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    VariableParameter();
  }
}

void DefaultParameter() {
  VariableParameter();
  Expect(LexemeType::kOperator, L"=");
  GetNext();
  Expression();
}

void DefaultParameterList() {
  DefaultParameter();
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    DefaultParameter();
  }
}

void ParameterList() {
  // Походу, с такой реализацией, 3 верхних функции не нужны
  // Иначе я не смог придумать, как реализовать спуск без больших костылей
  // Я их пока не стал удалять - мало ли...

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
  Type();
  Expect(LexemeType::kIdentifier);
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  ParameterList();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
}

void Type() {
  if (IsLexeme(LexemeType::kReserved, L"const")) {
    GetNext();
  }
  TypeNoConst();
  if (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
  }
}

void TypeNoConst() {
  Expect(LexemeType::kVariableType);
}

void If() {
  Expect(LexemeType::kReserved, L"if");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
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
  if (IsLexeme(LexemeType::kReserved, L"else")) {
    GetNext();
    Block();
  }
}

void For() {
  Expect(LexemeType::kReserved, L"for");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  EpsExpression();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  Expression();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  EpsExpression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Block();
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
  Identifier();
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

void Priority1() {
  Priority2();
  while (IsLexeme(LexemeType::kOperator, L"=")
    || IsLexeme(LexemeType::kOperator, L"<<=")
    || IsLexeme(LexemeType::kOperator, L">>=")
    || IsLexeme(LexemeType::kOperator, L"+=")
    || IsLexeme(LexemeType::kOperator, L"-=")
    || IsLexeme(LexemeType::kOperator, L"*=")
    || IsLexeme(LexemeType::kOperator, L"**=")
    || IsLexeme(LexemeType::kOperator, L"/=")
    || IsLexeme(LexemeType::kOperator, L"^=")
    || IsLexeme(LexemeType::kOperator, L"&")
    || IsLexeme(LexemeType::kOperator, L"|=")
    || IsLexeme(LexemeType::kOperator, L"&&=")
    || IsLexeme(LexemeType::kOperator, L"||=")
    || IsLexeme(LexemeType::kOperator, L"%=")) {
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
  }
  else if (IsLexeme(LexemeType::kBracket, L"[")) {
    GetNext();
    Expression();
    Expect(LexemeType::kBracket, L"]");
    GetNext();
  }
  else if (IsLexeme(LexemeType::kOperator, L".")) {
    GetNext();
    Priority16();
  }
}
void Priority16() {
  if (IsLexeme(LexemeType::kNumericLiteral)
    || IsLexeme(LexemeType::kIdentifier)
    || IsLexeme(LexemeType::kStringLiteral)) {
    GetNext();
  }
  else {
    Expect(LexemeType::kParenthesis, L"(")
    GetNext();
    Expression();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
  }
}
