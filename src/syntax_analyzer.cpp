#include "syntax_analyzer.hpp"
#include "exceptions.hpp"
#include "lexeme.hpp"
#include "TID.hpp"
#include "operators.hpp"
#include <memory>
#include <string>
#include <vector>
#include "casts.hpp"
#include "operations.hpp"
#include "warnings.hpp"
#include "logging.hpp"

#define DEBUG_ACTIVE 0

#if defined(DEBUG_ACTIVE) && DEBUG_ACTIVE
#include <iostream>
void debug_out() { std::wcerr << std::endl; }
template <typename Head, typename... Tail> void debug_out(Head H, Tail... T) { std::wcerr << L" " << H; debug_out(T...);}
#define debug(...) std::wcerr << "[" << __LINE__ << "]:", debug_out(__VA_ARGS__)
#else
#define debug(...) 42
#endif

#define STRONG_TYPES 0
#if defined(STRONG_TYPES) && STRONG_TYPES
#define CanCast CanCastLossless
#else
#endif

std::vector<std::shared_ptr<TIDVariableType>> scope_return_type;
uint32_t surrounding_loop_count = 0;

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
  scope_return_type.push_back(GetPrimitiveVariableType(PrimitiveVariableType::kInt32));
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
void Block(bool add_scope);
void Keyword();
std::shared_ptr<TIDVariableType> Expression();
std::shared_ptr<TIDVariableType> EpsExpression();
void Struct();
std::pair<std::wstring, std::shared_ptr<TIDVariableType>> VariableIdentifier(std::shared_ptr<TIDVariableType> & type);
std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> Definition();
void DefinitionAddToScope();
std::pair<std::wstring, std::shared_ptr<TIDVariableType>> VariableParameter();
std::pair<std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>>,
  std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>>> ParameterList();
std::shared_ptr<TIDVariableType> Function(const std::wstring & name,
                                          const std::shared_ptr<TIDVariableType> & return_type);
std::shared_ptr<TIDVariableType> Type(bool complete_type = false);
std::shared_ptr<TIDVariableType> TypeNoConst();
void If();
void Elif();
void Else();
void For();
void Foreach();
void While();
void DoWhile();
/*void Try();
void Throw();*/
void Continue();
void Break();
void Return();
void FunctionCall(const std::shared_ptr<TIDVariableType> & type);
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


void ExpectToBeAbleToCastTo(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  if (!from && !to) return;
  if (!from || !to || !CanCast(SetParamsToType(from, false, false),
                                                         SetParamsToType(to, false, false)))
    throw TypeMismatch(lexeme, to, from);
}

void Program() {
  debug("Program");
  while (!eof) {
    Action();
  }
}

void Action() {
  debug("Action");
  debug(lexeme.GetValue());
  if (IsLexeme(LexemeType::kReserved) || IsLexeme(LexemeType::kVariableType) ||
      (IsLexeme(LexemeType::kIdentifier) && tid.GetComplexStruct(lexeme.GetValue()) != nullptr))
    Keyword();
  else if (IsLexeme(LexemeType::kPunctuation, L"{"))
    Block(true);
  else {
    EpsExpression();
    Expect(LexemeType::kPunctuation, L";");
    GetNext();
  }
  debug("Exited action");
}

void Block(bool add_scope) {
  debug("Block");
  if (add_scope) tid.AddScope();
  if (!IsLexeme(LexemeType::kPunctuation, L"{")) {
    Action();
    if (add_scope) tid.RemoveScope();
    return;
  }
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
    debug("inside of a block");
    Action();
  }
  Expect(LexemeType::kPunctuation, L"}");
  if (add_scope) tid.RemoveScope();
  GetNext();
}

void Keyword() {
  debug("Keyword");
  if (!IsLexeme(LexemeType::kReserved) && !IsLexeme(LexemeType::kVariableType)) {
    if (!IsLexeme(LexemeType::kIdentifier))
      Expect(LexemeType::kReserved);
    if (tid.GetComplexStruct(lexeme.GetValue()) == nullptr)
      throw UndeclaredIdentifier(lexeme);
  }
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
  else if (IsLexeme(L"struct")) {
    tid.RemoveScope();
    Struct();
    return;
  } /* else if (IsLexeme(L"try"))
    Try();
  else if (IsLexeme(L"throw"))
    Throw(); */
  else {
    tid.RemoveScope();
    DefinitionAddToScope();
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
  std::wstring name = lexeme.GetValue();
  GetNext();
  Expect(LexemeType::kPunctuation, L"{");
  GetNext();

  std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> result;
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
    auto vars = Definition();
    for (auto & p : vars)
      result.push_back(p);
  }
  std::shared_ptr<TIDVariableType> complex_type = std::make_shared<TIDComplexVariableType>(name, result);
  tid.AddComplexStruct(lexeme, name, complex_type);

  Expect(LexemeType::kPunctuation, L"}");
  GetNext();
}

std::pair<std::wstring, std::shared_ptr<TIDVariableType>> VariableIdentifier(std::shared_ptr<TIDVariableType> & type) {
  debug("Variable Identifier");
  uint32_t pointer_count = 0;
  while (IsLexeme(LexemeType::kOperator, L"*")) {
    ++pointer_count;
    GetNext();
  }
  std::wstring name;
  std::shared_ptr<TIDVariableType> ptr;
  if (IsLexeme(LexemeType::kParenthesis, L"(")) {
    GetNext();
    std::tie(name, ptr) = VariableIdentifier(type);
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
  } else {
    Expect(LexemeType::kIdentifier);
    name = lexeme.GetValue();
    GetNext();
    ptr = type;
  }
  while (pointer_count--)
    ptr = DerivePointerFromType(ptr);
  while (IsLexeme(LexemeType::kBracket, L"[")) {
    GetNext();
    if (!IsLexeme(LexemeType::kBracket, L"]")) {
      auto ind_type = Expression();
      ExpectToBeAbleToCastTo(ind_type,
        SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
      if (!CanCastLossless(ind_type, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true)))
        log::warning(Downcast(lexeme, ind_type, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true)));
    }
    Expect(LexemeType::kBracket, L"]");
    GetNext();
    ptr = DeriveArrayFromType(ptr);
  }
  return { name, ptr };
}

std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> Definition() {
  debug("Definition");
  std::shared_ptr<TIDVariableType> type = Type();
  std::shared_ptr<TIDVariableType> var_type;
  std::wstring name;
  std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> result;
  if (IsLexeme(LexemeType::kIdentifier)) {
    name = lexeme.GetValue();
    GetNext();
    if (IsLexeme(LexemeType::kParenthesis)) {
      std::shared_ptr<TIDVariableType> func_type = Function(name, type);
      return { { name, func_type } };
    }

    if (type == nullptr)
      throw VoidNotExpected(lexeme);
    var_type = type;
    while (IsLexeme(LexemeType::kBracket, L"[")) {
      GetNext();
      if (!IsLexeme(LexemeType::kBracket, L"]")) {
        auto ind_type = Expression();
        ExpectToBeAbleToCastTo(ind_type,
          SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
        if (!CanCastLossless(ind_type, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true)))
          log::warning(Downcast(lexeme, ind_type, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true)));
      }
      Expect(LexemeType::kBracket, L"]");
      GetNext();
      var_type = DeriveArrayFromType(var_type);
    }
  } else {
    tie(name, var_type) = VariableIdentifier(type);
  }
  result.emplace_back(name, var_type);

  if (IsLexeme(LexemeType::kOperator, L"=")) {
    GetNext();
    auto expr_type = Expression();
    ExpectToBeAbleToCastTo(expr_type, var_type);
    if (!CanCastLossless(expr_type, var_type))
      log::warning(Downcast(lexeme, expr_type, var_type));
  }
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    tie(name, var_type) = VariableIdentifier(type);
    result.emplace_back(name, var_type);
    if (IsLexeme(LexemeType::kOperator, L"=")) {
      GetNext();
      auto expr_type = Expression();
      ExpectToBeAbleToCastTo(expr_type, var_type);
      if (!CanCastLossless(expr_type, var_type))
        log::warning(Downcast(lexeme, expr_type, var_type));
    }
  }
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  return result;
}

void DefinitionAddToScope() {
  auto vars = Definition();
  for (auto & [name, type] : vars) {
    if (type->GetType() == VariableType::kFunction) {
      std::shared_ptr<TIDVariableType> existing_type = tid.GetVariable(name);
      if (existing_type) {
        bool equal = existing_type->GetType() == VariableType::kFunction;
        if (equal) {
          std::shared_ptr<TIDFunctionVariableType> func_type = std::static_pointer_cast<TIDFunctionVariableType>(type);
          std::shared_ptr<TIDFunctionVariableType> existing_func_type =
                std::static_pointer_cast<TIDFunctionVariableType>(existing_type);
          equal &= func_type->GetReturnType() == existing_func_type->GetReturnType();
          equal &= func_type->GetParameters().size() == existing_func_type->GetParameters().size();
          equal &= func_type->GetDefaultParameters().size() == existing_func_type->GetDefaultParameters().size();
          if (equal) {
            for (size_t index = 0; index < func_type->GetParameters().size(); ++index)
              equal &= func_type->GetParameters()[index] == existing_func_type->GetParameters()[index];
            for (size_t index = 0; index < func_type->GetDefaultParameters().size(); ++index)
              equal &= func_type->GetDefaultParameters()[index] == existing_func_type->GetDefaultParameters()[index];
          }
        }
        if (equal) continue;
        else
          throw TypeMismatch(lexeme, existing_type, type);
      }
    }
    tid.AddVariable(lexeme, name, type);
  }
}

std::pair<std::wstring, std::shared_ptr<TIDVariableType>> VariableParameter() {
  std::shared_ptr<TIDVariableType> type = Type();
  if (type == nullptr)
    throw VoidNotExpected(lexeme);
  return VariableIdentifier(type);
}

std::pair<std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>>,
  std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>>> ParameterList() {
  bool started_default = false;
  std::vector<std::pair<std::wstring, std::shared_ptr<TIDVariableType>>> params, default_params;

  if (IsLexeme(LexemeType::kParenthesis, L")"))
    return { params, default_params };

  while (true) {
    auto var = VariableParameter();
    if (IsLexeme(LexemeType::kOperator, L"="))
      started_default = true;
    if (started_default) {
      default_params.push_back(var);
      if (!IsLexeme(LexemeType::kOperator, L"=")) {
        throw ExpectedDefaultParameter(lexeme);
      }
      GetNext();
      auto param_type = Expression();
      ExpectToBeAbleToCastTo(param_type, var.second);
      if (!CanCastLossless(param_type, var.second))
        log::warning(Downcast(lexeme, param_type, var.second));
    } else {
      params.push_back(var);
    }
    if (!IsLexeme(LexemeType::kPunctuation, L",")) break;
    GetNext();
  }

  return { params, default_params };
}

std::shared_ptr<TIDVariableType> Function(const std::wstring & name,
                                          const std::shared_ptr<TIDVariableType> & return_type) {
  debug("Function");
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  auto parameters = ParameterList();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();

  std::vector<std::shared_ptr<TIDVariableType>>
      params_tid(parameters.first.size()), default_params_tid(parameters.second.size());
  for (size_t i = 0; i < parameters.first.size(); ++i) params_tid[i] = parameters.first[i].second;
  for (size_t i = 0; i < parameters.second.size(); ++i) default_params_tid[i] = parameters.second[i].second;
  std::shared_ptr<TIDVariableType> func_type =
      std::make_shared<TIDFunctionVariableType>(return_type, params_tid, default_params_tid);

  if (!IsLexeme(LexemeType::kPunctuation, L";")) {
    tid.AddScope();
    scope_return_type.push_back(return_type);
    tid.AddVariable(lexeme, name, func_type);
    for (auto & [var_name, var_type] : parameters.first)
      tid.AddVariable(lexeme, var_name, var_type);
    for (auto & [var_name, var_type] : parameters.second)
      tid.AddVariable(lexeme, var_name, var_type);
    auto temp_value = surrounding_loop_count;
    surrounding_loop_count = 0;
    Block(false);
    surrounding_loop_count = temp_value;
    scope_return_type.pop_back();
    tid.RemoveScope();
  } else {
    GetNext();
  }
  debug("Exited Function");
  // also update scope_return_type
  return func_type;
}

std::shared_ptr<TIDVariableType> Type(bool complete_type) {
  debug("Type");
  bool _const = false, _ref = false;
  if (IsLexeme(LexemeType::kReserved, L"const")) {
    GetNext();
    _const = true;
  }
  std::shared_ptr<TIDVariableType> ptr = TypeNoConst();
  if (complete_type) {
    while (IsLexeme(LexemeType::kOperator, L"*") || IsLexeme(LexemeType::kBracket, L"[")) {
      if (IsLexeme(LexemeType::kOperator, L"*")) {
        GetNext();
        ptr = DerivePointerFromType(ptr);
      } else {
        GetNext();
        Expect(LexemeType::kBracket, L"]");
        GetNext();
        ptr = DeriveArrayFromType(ptr);
      }
    }
  }
  if (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
    _ref = true;
  }
  if (ptr)
    ptr = SetParamsToType(ptr, _const, _ref);
  else if (_const || _ref)
    throw VoidNotExpected(lexeme);

  return ptr;
}

std::shared_ptr<TIDVariableType> TypeNoConst() {
  if (!IsLexeme(LexemeType::kVariableType) && !IsLexeme(LexemeType::kIdentifier))
    Expect(LexemeType::kVariableType);
  std::shared_ptr<TIDVariableType> ptr = nullptr;
  if (lexeme.GetValue() != L"void") {
    PrimitiveVariableType primitive_type = FromWstringToPrimitiveType(lexeme.GetValue());
    if (primitive_type == PrimitiveVariableType::kUnknown) {
      ptr = tid.GetComplexStruct(lexeme.GetValue());
      if (!ptr)
        throw UndeclaredIdentifier(lexeme);
    } else {
      ptr = GetPrimitiveVariableType(primitive_type);
    }
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
  ExpectToBeAbleToCastTo(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool));
  if (!CanCastLossless(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)))
    log::warning(Downcast(lexeme, type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)));
  Block(true);
  Elif();
  if (IsLexeme(LexemeType::kReserved, L"else"))
    Else();
}

void Elif() {
  while (IsLexeme(LexemeType::kReserved, L"elif")) {
    GetNext();
    Expect(LexemeType::kParenthesis, L"(");
    GetNext();
    std::shared_ptr<TIDVariableType> type = Expression();
    ExpectToBeAbleToCastTo(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool));
    if (!CanCastLossless(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)))
      log::warning(Downcast(lexeme, type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)));
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    Block(true);
  }
}

void Else() {
  Expect(LexemeType::kReserved, L"else");
  GetNext();
  Block(true);
}

void For() {
  debug("For");
  Expect(LexemeType::kReserved, L"for");
  GetNext();
  tid.AddScope();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    DefinitionAddToScope();
  else
    GetNext();
  std::shared_ptr<TIDVariableType> type = EpsExpression();
  if (type)
    ExpectToBeAbleToCastTo(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool));
  if (!CanCastLossless(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)))
    log::warning(Downcast(lexeme, type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)));
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  if (!IsLexeme(LexemeType::kParenthesis, L")"))
    Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  ++surrounding_loop_count;
  Block(false);
  --surrounding_loop_count;
  tid.RemoveScope();
  debug("Exited For");
}

void Foreach() {
  Expect(LexemeType::kReserved, L"foreach");
  GetNext();
  tid.AddScope();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  auto [iter_name, iter_type] = VariableParameter();
  Expect(LexemeType::kReserved, L"of");
  GetNext();
  std::shared_ptr<TIDVariableType> arr_type = Expression();
  Expect(LexemeType::kParenthesis, L")");
  if (arr_type->GetType() != VariableType::kArray)
    throw TypeNotIterable(lexeme); // TODO: thrown lexeme is not the one needed lol
  auto expected_type = std::static_pointer_cast<TIDArrayVariableType>(arr_type)->GetValue();
  if (expected_type != iter_type)
    throw TypeMismatch(lexeme, expected_type, iter_type);
  tid.AddVariable(lexeme, iter_name, iter_type);
  GetNext();
  ++surrounding_loop_count;
  Block(false);
  --surrounding_loop_count;
  tid.RemoveScope();
}

void While() {
  Expect(LexemeType::kReserved, L"while");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  auto type = Expression();
  ExpectToBeAbleToCastTo(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool));
  if (!CanCastLossless(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)))
    log::warning(Downcast(lexeme, type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)));
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  ++surrounding_loop_count;
  Block(true);
  --surrounding_loop_count;
}

void DoWhile() {
  Expect(LexemeType::kReserved, L"do");
  GetNext();
  Block(true);
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  Expect(LexemeType::kReserved, L"while");
  GetNext();
  auto type = Expression();
  ExpectToBeAbleToCastTo(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool));
  if (!CanCastLossless(type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)))
    log::warning(Downcast(lexeme, type, GetPrimitiveVariableType(PrimitiveVariableType::kBool)));
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  ++surrounding_loop_count;
  GetNext();
  --surrounding_loop_count;
}

// Maybe if we had extra time...
/*void Try() {
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
}*/

void Continue() {
  Expect(LexemeType::kReserved, L"continue");
  if (surrounding_loop_count == 0)
    throw LoopInstructionsOutsideOfLoop(lexeme);
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Break() {
  Expect(LexemeType::kReserved, L"break");
  if (surrounding_loop_count == 0)
    throw LoopInstructionsOutsideOfLoop(lexeme);
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
}

void Return() {
  Expect(LexemeType::kReserved, L"return");
  GetNext();
  auto type = EpsExpression();
  Expect(LexemeType::kPunctuation, L";");
  ExpectToBeAbleToCastTo(type, scope_return_type.back());
  if (!CanCastLossless(type, scope_return_type.back()))
    log::warning(Downcast(lexeme, type, scope_return_type.back()));
  GetNext();
}

void FunctionCall(const std::shared_ptr<TIDVariableType> & type) {
  debug("Function Call");
  if (type->GetType() != VariableType::kFunction)
    throw ExpectedFunction(type);
  std::shared_ptr<TIDFunctionVariableType> func_type = std::static_pointer_cast<TIDFunctionVariableType>(type);
  const std::vector<std::shared_ptr<TIDVariableType>> & params = func_type->GetParameters();
  const std::vector<std::shared_ptr<TIDVariableType>> & default_params = func_type->GetDefaultParameters();

  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::vector<std::shared_ptr<TIDVariableType>> provided;
  if (!IsLexeme(LexemeType::kParenthesis, L")")) {
    provided.push_back(Expression());
    while (IsLexeme(LexemeType::kPunctuation, L",")) {
      GetNext();
      provided.push_back(Expression());
    }
  }
  Expect(LexemeType::kParenthesis, L")");
  GetNext();

  size_t param_index = 0, default_param_index = 0;
  for (const std::shared_ptr<TIDVariableType> & provided_type : provided) {
    if (param_index == params.size()) {
      if (default_param_index == default_params.size() || !CanCast(
          SetConstToType(provided_type, false), default_params[default_param_index++]))
        throw FunctionParameterListDoesNotMatch(lexeme, func_type, provided);
    } else {
      if (!CanCast(SetConstToType(provided_type, false), params[param_index++]))
        throw FunctionParameterListDoesNotMatch(lexeme, func_type, provided);
    }
  }
  if (param_index < params.size())
    throw FunctionParameterListDoesNotMatch(lexeme, func_type, provided);
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
    std::shared_ptr<TIDVariableType> rhs = st.back().second;
    BinaryOperator op = st.back().first;
    st.pop_back();
    std::shared_ptr<TIDVariableType> lhs = st.back().second;
    BinaryOperator next_op = st.back().first;
    st.pop_back();
    st.emplace_back(next_op, BinaryOperation(lhs, op, rhs, lexeme));
  }
  return st[0].second;
}

std::shared_ptr<TIDVariableType> Priority2() {
  auto type = Priority3();
  while (IsLexeme(LexemeType::kOperator, L"&&")) {
    GetNext();
    auto new_type = Priority3();
    type = BinaryOperation(type, BinaryOperator::kLogicalAnd, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority3() {
  auto type = Priority4();
  while (IsLexeme(LexemeType::kOperator, L"||")) {
    GetNext();
    auto new_type = Priority4();
    type = BinaryOperation(type, BinaryOperator::kLogicalOr, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority4() {
  auto type = Priority5();
  while (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
    auto new_type = Priority5();
    type = BinaryOperation(type, BinaryOperator::kBitwiseAnd, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority5() {
  auto type = Priority6();
  while (IsLexeme(LexemeType::kOperator, L"|")) {
    GetNext();
    auto new_type = Priority6();
    type = BinaryOperation(type, BinaryOperator::kBitwiseOr, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority6() {
  auto type = Priority7();
  while (IsLexeme(LexemeType::kOperator, L"^")) {
    GetNext();
    auto new_type = Priority7();
    type = BinaryOperation(type, BinaryOperator::kBitwiseXor, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority7() {
  auto type = Priority8();
  while (IsLexeme(LexemeType::kOperator, L"==")
    || IsLexeme(LexemeType::kOperator, L"!=")) {
    BinaryOperator op = IsLexeme(LexemeType::kOperator, L"==") ? BinaryOperator::kEqual : BinaryOperator::kNotEqual;
    GetNext();
    auto new_type = Priority8();
    type = BinaryOperation(type, op, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority8() {
  auto type = Priority9();
  while (IsLexeme(LexemeType::kOperator, L"<")
    || IsLexeme(LexemeType::kOperator, L"<=")
    || IsLexeme(LexemeType::kOperator, L">")
    || IsLexeme(LexemeType::kOperator, L">=")) {
    BinaryOperator op = BinaryOperator::kLess;
    if (IsLexeme(LexemeType::kOperator, L"<="))
      op = BinaryOperator::kLessOrEqual;
    else if (IsLexeme(LexemeType::kOperator, L">"))
      op = BinaryOperator::kMore;
    else if (IsLexeme(LexemeType::kOperator, L">="))
      op = BinaryOperator::kMoreOrEqual;
    GetNext();
    auto new_type = Priority9();
    type = BinaryOperation(type, op, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority9() {
  auto type = Priority10();
  while (IsLexeme(LexemeType::kOperator, L"<<")
    || IsLexeme(LexemeType::kOperator, L">>")) {
    BinaryOperator op = IsLexeme(LexemeType::kOperator, L"<<") ? BinaryOperator::kBitwiseShiftLeft
        : BinaryOperator::kBitwiseShiftRight;
    GetNext();
    auto new_type = Priority10();
    type = BinaryOperation(type, op, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority10() {
  auto type = Priority11();
  while (IsLexeme(LexemeType::kOperator, L"+")
    || IsLexeme(LexemeType::kOperator, L"-")) {
    BinaryOperator op = IsLexeme(LexemeType::kOperator, L"+") ? BinaryOperator::kAddition
        : BinaryOperator::kSubtraction;
    GetNext();
    auto new_type = Priority11();
    type = BinaryOperation(type, op, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority11() {
  auto type = Priority12();
  while (IsLexeme(LexemeType::kOperator, L"*")
    || IsLexeme(LexemeType::kOperator, L"/")
    || IsLexeme(LexemeType::kOperator, L"%")) {
    BinaryOperator op = BinaryOperator::kModulus;
    if (IsLexeme(LexemeType::kOperator, L"*")) {
      op = BinaryOperator::kMultiplication;
    } else if (IsLexeme(LexemeType::kOperator, L"/")) {
      op = BinaryOperator::kDivision;
    }
    GetNext();
    auto new_type = Priority12();
    type = BinaryOperation(type, op, new_type, lexeme);
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority12() {
  std::vector<UnaryPrefixOperator> ops;
  while (true) {
    UnaryPrefixOperator op = UnaryPrefixOperator::kUnknown;
    if (IsLexeme(LexemeType::kOperator, L"+"))
      op = UnaryPrefixOperator::kPlus;
    else if (IsLexeme(LexemeType::kOperator, L"-"))
      op = UnaryPrefixOperator::kMinus;
    else if (IsLexeme(LexemeType::kOperator, L"++"))
      op = UnaryPrefixOperator::kIncrement;
    else if (IsLexeme(LexemeType::kOperator, L"--"))
      op = UnaryPrefixOperator::kDecrement;
    else if (IsLexeme(LexemeType::kOperator, L"!"))
      op = UnaryPrefixOperator::kInvert;
    else if (IsLexeme(LexemeType::kOperator, L"~"))
      op = UnaryPrefixOperator::kTilda;
    else if (IsLexeme(LexemeType::kOperator, L"&"))
      op = UnaryPrefixOperator::kAddressOf;
    else if (IsLexeme(LexemeType::kOperator, L"*"))
      op = UnaryPrefixOperator::kDereference;
    else
      break;
    GetNext();
    ops.push_back(op);
  }
  auto type = Priority13();
  while (!ops.empty()) {
    type = UnaryPrefixOperation(ops.back(), type, lexeme);
    ops.pop_back();
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority13() {
  auto type = Priority14();
  while (true) {
    if (IsLexeme(LexemeType::kBracket, L"[")) {
      if (!type || type->GetType() != VariableType::kArray)
        throw TypeNotIndexed(lexeme);
      GetNext();
      if (!IsLexeme(LexemeType::kBracket, L"]")) {
        auto index_type = Expression();
        ExpectToBeAbleToCastTo(index_type, GetPrimitiveVariableType(PrimitiveVariableType::kUint32));
        if (!CanCastLossless(index_type, GetPrimitiveVariableType(PrimitiveVariableType::kUint32)))
          log::warning(Downcast(lexeme, index_type, GetPrimitiveVariableType(PrimitiveVariableType::kUint32)));
      }
      Expect(LexemeType::kBracket, L"]");
      GetNext();
      type = std::static_pointer_cast<TIDArrayVariableType>(type)->GetValue();
    } else if (IsLexeme(LexemeType::kParenthesis, L"(")) {
      if (!type || type->GetType() != VariableType::kFunction)
        throw TypeNotCallable(lexeme);
      FunctionCall(type);
      type = std::static_pointer_cast<TIDFunctionVariableType>(type)->GetReturnType();
    } else if (IsLexeme(LexemeType::kOperator, L".")) {
      if (!type || type->GetType() != VariableType::kComplex)
        throw TypeNoMembers(lexeme);
      GetNext();
      Expect(LexemeType::kIdentifier);
      std::wstring member_name = lexeme.GetValue();
      auto new_type = std::static_pointer_cast<TIDComplexVariableType>(type)->GetField(member_name);
      if (new_type == nullptr)
        throw TypeUnknownMember(lexeme);
      GetNext();
      type = SetParamsToType(new_type, type->IsConst(), true);
    } else if (IsLexeme(LexemeType::kOperator, L"++") || IsLexeme(LexemeType::kOperator, L"--")) {
      UnaryPostfixOperator op = IsLexeme(LexemeType::kOperator, L"++") ? UnaryPostfixOperator::kIncrement
        : UnaryPostfixOperator::kDecrement;
      GetNext();
      type = UnaryPostfixOperation(type, op, lexeme);
    } else if (IsLexeme(LexemeType::kReserved, L"as")) {
      GetNext();
      std::shared_ptr<TIDVariableType> new_type = Type(true);
      ExpectToBeAbleToCastTo(type, new_type);
      ExpectToBeAbleToCastTo(type, new_type);
      type = new_type;
    } else
      break;
  }
  return type;
}

std::shared_ptr<TIDVariableType> Priority14() {
  /*IsLexeme(LexemeType::kNumericLiteral)
    || IsLexeme(LexemeType::kIdentifier)
    || IsLexeme(LexemeType::kStringLiteral)
    || IsLexeme(LexemeType::kReserved, L"true") || IsLexeme(LexemeType::kReserved, L"false")*/
  if (IsLexeme(LexemeType::kParenthesis, L"(")) {
    GetNext();
    auto type = Expression();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    return type;
  } else {
    std::shared_ptr<TIDVariableType> type;
    if (IsLexeme(LexemeType::kNumericLiteral))
      type = SetConstToType(GetPrimitiveVariableType(lexeme.GetValue().find(L".") == std::wstring::npos ?
          PrimitiveVariableType::kInt8 : PrimitiveVariableType::kF32), true);
    else if (IsLexeme(LexemeType::kIdentifier)) {
      type = tid.GetVariable(lexeme.GetValue());
      if (type == nullptr)
        throw UndeclaredIdentifier(lexeme);
    } else if (IsLexeme(LexemeType::kStringLiteral))
      type = SetConstToType(DeriveArrayFromType(GetPrimitiveVariableType(PrimitiveVariableType::kChar)), true);
    else
      type = SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true);
    if (!type)
      throw ExpectedExpression(lexeme);
    GetNext();
    return type;
  }
}
