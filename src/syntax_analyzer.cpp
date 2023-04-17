#include "syntax_analyzer.hpp"
#include "exceptions.hpp"
#include "lexeme.hpp"
#include "TID.hpp"
#include "operators.hpp"
#include <memory>
#include <vector>

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
TID tid;

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
std::shared_ptr<TIDVariableType> Expression();
void Struct();
std::shared_ptr<TIDVariableType> VariableIdentifier(std::shared_ptr<TIDVariableType> & type);
void Definition();
std::shared_ptr<TIDVariableType> VariableParameter();
void ParameterList();
void LambdaFunction();
void Function();
std::shared_ptr<TIDVariableType> Type();
std::shared_ptr<TIDVariableType> TypeNoConst();
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
std::shared_ptr<TIDVariableType> Return();
// TODO: should return types of arguments passed down so caller can check if they are correct
void FunctionCall();
std::shared_ptr<TIDVariableType> Priority1();
std::shared_ptr<TIDVariableType> Priority2();
std::shared_ptr<TIDVariableType> Priority3();
std::shared_ptr<TIDVariableType> Priority4();
std::shared_ptr<TIDVariableType> Priority5();
std::shared_ptr<TIDVariableType> Priority6();
std::shared_ptr<TIDVariableType> Priority7();
std::shared_ptr<TIDVariableType> Priority8();
std::shared_ptr<TIDVariableType> Priority9();
std::shared_ptr<TIDVariableType> Priority10();
std::shared_ptr<TIDVariableType> Priority11();
std::shared_ptr<TIDVariableType> Priority12();
std::shared_ptr<TIDVariableType> Priority13();
std::shared_ptr<TIDVariableType> Priority14();
std::shared_ptr<TIDVariableType> Priority15();
std::shared_ptr<TIDVariableType> Priority16();

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
  tid.AddScope();
  if (!IsLexeme(LexemeType::kPunctuation, L"{")) {
    Action();
    tid.RemoveScope();
    return;
  }
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
    Action();
  }
  Expect(LexemeType::kPunctuation, L"}");
  tid.RemoveScope();
  GetNext();
}

void Keyword() {
  debug("Keyword");
  Expect(LexemeType::kReserved, LexemeType::kVariableType);
  tid.AddScope();
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
  else {
    tid.RemoveScope();
    Definition();
    return;
  }
  tid.RemoveScope(); // hacky workaround so I don't have to insert AddScope and RemoveScope inside every "keyword"
}

std::shared_ptr<TIDVariableType> Expression() {
  debug("Expression");
  return Priority1();
}

std::shared_ptr<TIDVariableType> EpsExpression() {
  debug("EpsExpression");
  if (IsLexeme(LexemeType::kPunctuation, L";"))
    return {};
  return Expression();
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

std::shared_ptr<TIDVariableType> VariableIdentifier(std::shared_ptr<TIDVariableType> & type) {
  debug("Variable Identifier");
  uint32_t pointer_count = 0;
  while (IsLexeme(LexemeType::kOperator, L"*")) {
    ++pointer_count;
    GetNext();
  }
  std::shared_ptr<TIDVariableType> ptr;
  if (IsLexeme(LexemeType::kParenthesis, L"(")) {
    GetNext();
    ptr = VariableIdentifier(type);
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
  } else {
    Expect(LexemeType::kIdentifier);
    GetNext();
    ptr = type;
  }
  while (pointer_count--)
    ptr = DerivePointerFromType(ptr);
  while (IsLexeme(LexemeType::kBracket, L"[")) {
    GetNext();
    Expression();
    Expect(LexemeType::kBracket, L"]");
    GetNext();
    ptr = DeriveArrayFromType(ptr);
  }
  return ptr;
}

void Definition() {
  debug("Definition");
  std::shared_ptr<TIDVariableType> type = Type();
  std::shared_ptr<TIDVariableType> variable_type;
  if (IsLexeme(LexemeType::kIdentifier)) {
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
  } else {
    variable_type = VariableIdentifier(type);
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

std::shared_ptr<TIDVariableType> VariableParameter() {
  std::shared_ptr<TIDVariableType> type = Type();
  std::shared_ptr<TIDVariableType> detailed_type = VariableIdentifier(type);
  return detailed_type;
}

void ParameterList() {
  bool started_default = false;

  while (true) {
    std::shared_ptr<TIDVariableType> var = VariableParameter();
    if (IsLexeme(LexemeType::kOperator, L"="))
      started_default = true;
    if (started_default) {
      Expect(LexemeType::kOperator, L"=");
      GetNext();
      Expression();
    }
    if (!IsLexeme(LexemeType::kPunctuation, L",")) break;
    GetNext();
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
  // Check if list of arguments is ok
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    Block();
  debug("Exited Function");
}

std::shared_ptr<TIDVariableType> Type() {
  debug("Type");
  bool _const = false, _ref = false;
  if (IsLexeme(LexemeType::kReserved, L"const")) {
    GetNext();
    _const = true;
  }
  std::shared_ptr<TIDVariableType> ptr = TypeNoConst();
  if (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
    _ref = true;
  }
  ptr->SetConst(_const);
  ptr->SetReference(_ref);
  return ptr;
}

std::shared_ptr<TIDVariableType> TypeNoConst() {
  if (!IsLexeme(LexemeType::kVariableType) && !IsLexeme(LexemeType::kIdentifier))
    Expect(LexemeType::kVariableType);
  std::shared_ptr<TIDVariableType> ptr;
  PrimitiveVariableType primitive_type = FromWstringToPrimitiveType(lexeme.GetValue());
  if (primitive_type == PrimitiveVariableType::kUnknown) {
    ptr = tid.GetComplexStruct(lexeme.GetValue());
    if (!ptr)
      throw UndeclaredIdentifier(lexeme);
  } else {
    ptr = GetPrimitiveVariableType(primitive_type);
  }
  GetNext();
  return ptr;
}

void If() {
  debug("If");
  Expect(LexemeType::kReserved, L"if");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  debug("lol");
  std::shared_ptr<TIDVariableType> type = Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  // Workaround: will throw exception if expression is not convertible to bool
  // TODO: change to casting system
  UnaryPrefixOperation(UnaryPrefixOperator::kInvert, type, lexeme);
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
  std::shared_ptr<TIDVariableType> type = EpsExpression();
  if (type)
    UnaryPrefixOperation(UnaryPrefixOperator::kInvert, type, lexeme); // TODO: change to casting system
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
  std::shared_ptr<TIDVariableType> iter_type = VariableParameter();
  Expect(LexemeType::kReserved, L"of");
  GetNext();
  std::shared_ptr<TIDVariableType> arr_type = Expression();
  Expect(LexemeType::kParenthesis, L")");
  if (arr_type->GetType() != VariableType::kArray)
    throw TypeNotIterable(lexeme); // TODO: thrown lexeme is not the one needed lol
  auto expected_type = std::static_pointer_cast<TIDArrayVariableType>(arr_type)->GetValue();
  if (expected_type != iter_type)
    throw TypeMismatch(lexeme, expected_type, iter_type);
  GetNext();
  Block();
}

void While() {
  Expect(LexemeType::kReserved, L"while");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  auto type = Expression();
  Expect(LexemeType::kParenthesis, L")");
  // TODO: change to casting system
  UnaryPrefixOperation(UnaryPrefixOperator::kInvert, type, lexeme);
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
  auto type = Expression();
  Expect(LexemeType::kParenthesis, L")");
  // TODO: change to casting system
  UnaryPrefixOperation(UnaryPrefixOperator::kInvert, type, lexeme);
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Try() {
  Expect(LexemeType::kReserved, L"try");
  GetNext();
  Block();
  Expect(LexemeType::kReserved, L"catch");
  // TODO: any type checking would be nice
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
  // TODO: wtf to do with throw
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

std::shared_ptr<TIDVariableType> Return() {
  Expect(LexemeType::kReserved, L"return");
  GetNext();
  auto type = Expression();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  return type;
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

const size_t signs1_sz = /*12*/ 11; // no power (**) 'cause it messes with pointers
/*const std::wstring signs1[signs1_sz] = {
  L"=", L"<<=", L">>=", L"+=", L"-=", L"*=", L"**=", L"/=",
  L"^=", L"&=", L"|=", L"%="
};*/
const BinaryOperator signs1[signs1_sz] = {
  BinaryOperator::kAssignment, BinaryOperator::kBitwiseShiftLeftAssignment, BinaryOperator::kBitwiseShiftRightAssignment,
  BinaryOperator::kAdditionAssignment, BinaryOperator::kSubtractionAssignment, BinaryOperator::kMultiplicationAssignment,
  BinaryOperator::kDivisionAssignment, BinaryOperator::kBitwiseXorAssignment, BinaryOperator::kBitwiseOrAssignment,
  BinaryOperator::kBitwiseAndAssignment, BinaryOperator::kModulusAssignment
};

std::shared_ptr<TIDVariableType> Priority1() {
  std::vector<std::pair<BinaryOperator, std::shared_ptr<TIDVariableType>>> st;
  st.emplace_back(BinaryOperator::kUnknown, Priority2());

  while (true) {
    BinaryOperator found_op = BinaryOperator::kUnknown;
    for (BinaryOperator op : signs1) {
      if (IsLexeme(ToString(op))) {
        found_op = op;
        break;
      }
    }
    if (found_op == BinaryOperator::kUnknown) break;
    GetNext();
    st.emplace_back(found_op, Priority2());
  }
  // all assignment operations are right to left so here we go
  while (st.size() >= 2) {
    // TODO: I didn't do binary type validation yet so if I add it now it will always fail
  }
  return st[0].second;
}

std::shared_ptr<TIDVariableType> Priority2() {
  Priority3();
  while (IsLexeme(LexemeType::kOperator, L"&&")) {
    GetNext();
    Priority3();
  }
}

std::shared_ptr<TIDVariableType> Priority3() {
  Priority4();
  while (IsLexeme(LexemeType::kOperator, L"||")) {
    GetNext();
    Priority4();
  }
}

std::shared_ptr<TIDVariableType> Priority4() {
  Priority5();
  while (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
    Priority5();
  }
}

std::shared_ptr<TIDVariableType> Priority5() {
  Priority6();
  while (IsLexeme(LexemeType::kOperator, L"|")) {
    GetNext();
    Priority6();
  }
}

std::shared_ptr<TIDVariableType> Priority6() {
  Priority7();
  while (IsLexeme(LexemeType::kOperator, L"^")) {
    GetNext();
    Priority7();
  }
}

std::shared_ptr<TIDVariableType> Priority7() {
  Priority8();
  while (IsLexeme(LexemeType::kOperator, L"==")
    || IsLexeme(LexemeType::kOperator, L"!=")) {
    GetNext();
    Priority8();
  }
}

std::shared_ptr<TIDVariableType> Priority8() {
  Priority9();
  while (IsLexeme(LexemeType::kOperator, L"<")
    || IsLexeme(LexemeType::kOperator, L"<=")
    || IsLexeme(LexemeType::kOperator, L">")
    || IsLexeme(LexemeType::kOperator, L">=")) {
    GetNext();
    Priority9();
  }
}

std::shared_ptr<TIDVariableType> Priority9() {
  Priority10();
  while (IsLexeme(LexemeType::kOperator, L"<<")
    || IsLexeme(LexemeType::kOperator, L">>")) {
    GetNext();
    Priority10();
  }
}

std::shared_ptr<TIDVariableType> Priority10() {
  Priority11();
  while (IsLexeme(LexemeType::kOperator, L"+")
    || IsLexeme(LexemeType::kOperator, L"-")) {
    GetNext();
    Priority11();
  }
}

std::shared_ptr<TIDVariableType> Priority11() {
  Priority12();
  while (IsLexeme(LexemeType::kOperator, L"*")
    || IsLexeme(LexemeType::kOperator, L"/")
    || IsLexeme(LexemeType::kOperator, L"%")) {
    GetNext();
    Priority12();
  }
}

std::shared_ptr<TIDVariableType> Priority12() {
  Priority13();
  while (IsLexeme(LexemeType::kOperator, L"**")) {
    GetNext();
    Priority13();
  }
}

std::shared_ptr<TIDVariableType> Priority13() {
  if (IsLexeme(LexemeType::kOperator, L"!")) {
    GetNext();
  }
  Priority14();
}

std::shared_ptr<TIDVariableType> Priority14() {
  UnaryPrefixOperator op = UnaryPrefixOperator::kUnknown;
  bool get_next = true;
  if (IsLexeme(LexemeType::kOperator, L"+"))
    op = UnaryPrefixOperator::kPlus;
  else if (IsLexeme(LexemeType::kOperator, L"-"))
    op = UnaryPrefixOperator::kMinus;
  else if (IsLexeme(LexemeType::kOperator, L"++"))
    op = UnaryPrefixOperator::kIncrement;
  else if (IsLexeme(LexemeType::kOperator, L"--"))
    op = UnaryPrefixOperator::kDecrement;
  else
    get_next = false;
  if (get_next) GetNext();
  auto type = Priority15();
  if (op == UnaryPrefixOperator::kUnknown) return type;
  else return UnaryPrefixOperation(op, type, lexeme);
}

std::shared_ptr<TIDVariableType> Priority15() {
  auto type = Priority16();
  if (IsLexeme(LexemeType::kOperator, L"++")
    || IsLexeme(LexemeType::kOperator, L"--")) {
    UnaryPostfixOperator op = IsLexeme(LexemeType::kOperator, L"++") ? UnaryPostfixOperator::kIncrement
      : UnaryPostfixOperator::kDecrement;
    GetNext();
    return UnaryPostfixOperation(type, op, lexeme);
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

std::shared_ptr<TIDVariableType> Priority16() {
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
