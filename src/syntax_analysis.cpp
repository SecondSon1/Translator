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
void Assignment();
void Expression();
void Struct();
void VariableIdentifier();
void VariableIdentifierList();
void Definition();
void VariableParameter();
void VariableParameterList();
void DefaultParameter();
void DefaultParameterList();
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
void Priority0();
void Sign0();
void Priority1();
void Sign1();
void Priority2();
void Sign2();
void Priority3();
void Sign3();
void Priority4();
void Sign4();
void Priority5();
void Sign5();
void Priority6();
void Sign6();
void Priority7();
void Sign7();
void Priority8();
void Sign8();
void Priority9();
void Sign9();
void Priority10();
void Sign10();
void Priority11();
void Sign11();
void Priority12();
void Sign12();
void Priority13();
void Sign13();
void Priority14();
void Sign14();
void Priority15();
void Sign15();
void Priority16();
void Sign16();


void Identifiers() {
  Expect(LexemeType::kIdentifier);
  GetNext();
  if (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    Identifiers();
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
  else
    Definition();
}

void Expression() {
  if (IsLexeme(LexemeType::kIdentifier) || IsLexeme(LexemeType::kNumericLiteral)
        || IsLexeme(LexemeType::kStringLiteral)) {
    GetNext();
    return;
  }
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

void VariableParameter();
void VariableParameterList();
void DefaultParameter();
void DefaultParameterList();
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
void Priority0();
void Sign0();
void Priority1();
void Sign1();
void Priority2();
void Sign2();
void Priority3();
void Sign3();
void Priority4();
void Sign4();
void Priority5();
void Sign5();
void Priority6();
void Sign6();
void Priority7();
void Sign7();
void Priority8();
void Sign8();
void Priority9();
void Sign9();
void Priority10();
void Sign10();
void Priority11();
void Sign11();
void Priority12();
void Sign12();
void Priority13();
void Sign13();
void Priority14();
void Sign14();
void Priority15();
void Sign15();
void Priority16();
void Sign16();
