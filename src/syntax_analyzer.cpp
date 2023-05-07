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
#include "generation.hpp"

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

std::map<std::wstring, std::shared_ptr<RPN>> func_rpn;
std::vector<std::shared_ptr<RPN>> rpn;

RPN PerformSyntaxAnalysis(const std::vector<Lexeme> & code) {
  if (code.empty()) return {};
  _lexemes = code;
  _lexeme_index = 0;
  lexeme = code[0];
  eof = false;
  scope_return_type.push_back(GetPrimitiveVariableType(PrimitiveVariableType::kInt32));
  std::vector<std::shared_ptr<TIDVariableType>> hoax_params;
  tid.AddVariable(lexeme, L"new", std::make_shared<TIDFunctionVariableType>(nullptr, hoax_params, hoax_params));
  tid.AddVariable(lexeme, L"delete", std::make_shared<TIDFunctionVariableType>(nullptr, hoax_params, hoax_params));
  tid.AddVariable(lexeme, L"size", std::make_shared<TIDFunctionVariableType>(nullptr, hoax_params, hoax_params));
  tid.AddVariable(lexeme, L"sizeof", std::make_shared<TIDFunctionVariableType>(nullptr, hoax_params, hoax_params));
  Program();
  RPN result;
  // TODO: merge RPN from all functions
  return result;
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
std::shared_ptr<TIDValue> Expression();
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
std::shared_ptr<TIDValue> New();
std::shared_ptr<TIDValue> Delete();
std::shared_ptr<TIDValue> Size();
std::shared_ptr<TIDValue> Sizeof();
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
void FunctionCall(const std::shared_ptr<TIDValue> & type);
std::shared_ptr<TIDValue> Priority1();
std::shared_ptr<TIDValue> Priority2();
std::shared_ptr<TIDValue> Priority3();
std::shared_ptr<TIDValue> Priority4();
std::shared_ptr<TIDValue> Priority5();
std::shared_ptr<TIDValue> Priority6();
std::shared_ptr<TIDValue> Priority7();
std::shared_ptr<TIDValue> Priority8();
std::shared_ptr<TIDValue> Priority9();
std::shared_ptr<TIDValue> Priority10();
std::shared_ptr<TIDValue> Priority11();
std::shared_ptr<TIDValue> Priority12();
std::shared_ptr<TIDValue> Priority13();
std::shared_ptr<TIDValue> Priority14();

void Cast(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to, [[maybe_unused]] bool suppress_downcast_warning = false) {
  if (!from && !to) return;
  if (!from || !to || !CanCast(SetParamsToType(from, false, false), SetParamsToType(to, false, false)))
    throw TypeMismatch(lexeme, to, from);

  if (!suppress_downcast_warning && !CanCastLossless(from, to))
    log::warning(Downcast(lexeme, from, to));
}
void PushNode(const RPNOperandNode & node) { rpn.back()->PushNode(node); }
void PushNode(const RPNReferenceOperandNode & node) { rpn.back()->PushNode(node); }
void PushNode(const RPNOperatorNode & node) { rpn.back()->PushNode(node); }
void LoadType(const std::shared_ptr<TIDVariableType> & type) {
  if (type->GetType() != VariableType::kComplex)
    PushNode(RPNOperatorNode(RPNOperatorType::kLoad, type->GetType() == VariableType::kPrimitive ?
          std::dynamic_pointer_cast<TIDPrimitiveVariableType>(type)->GetPrimitiveType() : PrimitiveVariableType::kUint64));
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
    if (!IsLexeme(LexemeType::kPunctuation, L";"))
      Expression();
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
    Struct();
    return;
  } /* else if (IsLexeme(L"try"))
    Try();
  else if (IsLexeme(L"throw"))
    Throw(); */
  else {
    DefinitionAddToScope();
    return;
  }
}

std::shared_ptr<TIDValue> Expression() {
  debug("Expression");
  return Priority1();
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
  tid.AddComplexStruct(lexeme, complex_type);

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
      std::shared_ptr<TIDValue> ind_val = Expression();
      Cast(ind_val->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
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
      std::shared_ptr<TIDValue> ind_val = Expression();
      Cast(ind_val->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
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
    std::shared_ptr<TIDValue> expr_val = Expression();
    Cast(expr_val->GetType(), var_type);
  }
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    tie(name, var_type) = VariableIdentifier(type);
    result.emplace_back(name, var_type);
    if (IsLexeme(LexemeType::kOperator, L"=")) {
      GetNext();
      auto expr_val = Expression();
      Cast(expr_val->GetType(), var_type);
    }
  }
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  return result;
}

void DefinitionAddToScope() {
  auto vars = Definition();
  for (auto & [name, type] : vars) {
    if (type->GetType() == VariableType::kFunction)
      continue;
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
      auto param_val = Expression();
      Cast(param_val->GetType(), var.second);
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
  std::shared_ptr<TIDVariableType> type =
      std::make_shared<TIDFunctionVariableType>(return_type, params_tid, default_params_tid);
  tid.AddVariable(lexeme, name, type);

  std::shared_ptr<TIDVariable> existing_var = tid.GetVariableFromCurrentScope(name);

  if (existing_var) {
    auto existing_type = existing_var->GetType();
    bool equal = existing_type->GetType() == VariableType::kFunction;
    if (equal) {
      std::shared_ptr<TIDFunctionVariableType> func_type = std::static_pointer_cast<TIDFunctionVariableType>(type);
      std::shared_ptr<TIDFunctionVariableType> existing_func_type = std::static_pointer_cast<TIDFunctionVariableType>(existing_type);
      equal &= func_type->GetReturnType() == existing_func_type->GetReturnType();
      equal &= func_type->GetParameters().size() == existing_func_type->GetParameters().size();
      equal &= func_type->GetDefaultParameters().size() == existing_func_type->GetDefaultParameters().size();
      if (equal) {
        for (size_t index = 0; index < func_type->GetParameters().size(); ++index)
          equal &= func_type->GetParameters()[index] == existing_func_type->GetParameters()[index];
        for (size_t index = 0; index < func_type->GetDefaultParameters().size(); ++index)
          equal &= func_type->GetDefaultParameters()[index] == existing_func_type->GetDefaultParameters()[index];
      }
      if (!equal)
        throw TypeMismatch(lexeme, existing_type, type);
    } else
      throw ConflictingNames(lexeme);
  }

  rpn.push_back(std::make_shared<RPN>());
  func_rpn[tid.GetVariable(name)->GetInternalName()] = rpn.back();

  if (!IsLexeme(LexemeType::kPunctuation, L";")) {
    tid.AddFunctionScope(return_type);
    scope_return_type.push_back(return_type);
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
    PushNode(RPNOperatorNode(RPNOperatorType::kReturn));
  }
  auto last = rpn.back()->GetNodes().back();
  if (last->GetNodeType() != NodeType::kOperator ||
      std::dynamic_pointer_cast<RPNOperatorNode>(last)->GetOperatorType() != RPNOperatorType::kReturn)
    PushNode(RPNOperatorNode(RPNOperatorType::kReturn));
  rpn.pop_back();

  debug("Exited Function");

  return type;
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
  std::shared_ptr<TIDValue> value = Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Cast(value->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
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
    std::shared_ptr<TIDValue> value = Expression();
    Cast(value->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
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
  std::shared_ptr<TIDValue> val;
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    val = Expression();
  if (val)
    Cast(val->GetType(), GetPrimitiveVariableType(PrimitiveVariableType::kBool));
  Expect(LexemeType::kPunctuation, L";");
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
  std::shared_ptr<TIDValue> arr_type = Expression();
  Expect(LexemeType::kParenthesis, L")");
  //                   LMFAO
  if (arr_type->GetType()->GetType() != VariableType::kArray)
    throw TypeNotIterable(lexeme); // TODO: thrown lexeme is not the one needed lol (it's actually everywhere)
  auto expected_type = std::static_pointer_cast<TIDArrayVariableType>(arr_type->GetType())->GetValue();
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
  std::shared_ptr<TIDValue> val = Expression();
  Cast(val->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
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
  std::shared_ptr<TIDValue> value = Expression();
  Cast(value->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
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
  std::shared_ptr<TIDValue> value;
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    value = Expression();
  Expect(LexemeType::kPunctuation, L";");
  Cast(value->GetType(), scope_return_type.back());
  GetNext();
}

std::shared_ptr<TIDValue> New() {
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDVariableType> type = Type(true);
  if (!type || (!IsLexeme(LexemeType::kParenthesis, L")") && !IsLexeme(LexemeType::kPunctuation, L",")))
    throw NewIncorrectUsage(lexeme, type);
  if (IsLexeme(LexemeType::kParenthesis, L")")) {
    GetNext();
    PushNode(RPNOperandNode(type->GetSize()));
    PushNode(RPNOperatorNode(RPNOperatorType::kNew));
    return std::make_shared<TIDTemporaryValue>(SetConstToType(DerivePointerFromType(type), true));
  } else {
    Expect(LexemeType::kPunctuation, L",");
    GetNext();
    std::shared_ptr<TIDValue> ind = Expression();
    // TODO: actually cast ind to uint32
    Cast(ind->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
    PushNode(RPNOperandNode(type->GetSize()));
    PushNode(RPNOperatorNode(RPNOperatorType::kMultiply));
    PushNode(RPNOperandNode(4));
    PushNode(RPNOperatorNode(RPNOperatorType::kAdd));
    PushNode(RPNOperatorNode(RPNOperatorType::kNew));
    return std::make_shared<TIDTemporaryValue>(SetConstToType(DeriveArrayFromType(type), true));
  }
}

std::shared_ptr<TIDValue> Delete() {
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  auto type = val->GetType();
  auto var_type = type->GetType();
  if (var_type != VariableType::kPointer && var_type != VariableType::kArray)
    throw DeleteIncorrectUsage(lexeme, val->GetType());
  GetNext();
  PushNode(RPNOperandNode(type->GetSize()));
  if (var_type == VariableType::kArray) {
    PushNode(RPNOperandNode(type->GetSize()));
    PushNode(RPNOperatorNode(RPNOperatorType::kMultiply));
    PushNode(RPNOperandNode(4));
    PushNode(RPNOperatorNode(RPNOperatorType::kAdd));
  }
  PushNode(RPNOperatorNode(RPNOperatorType::kDelete));
  return std::make_shared<TIDTemporaryValue>(nullptr);
}

std::shared_ptr<TIDValue> Size() {
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  auto type = val->GetType();
  if (type->GetType() != VariableType::kArray)
    throw SizeIncorrectUsage(lexeme, val->GetType());
  GetNext();
  return std::make_shared<TIDTemporaryValue>(SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
}

std::shared_ptr<TIDValue> Sizeof() {
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDVariableType> type = Type(true);
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  return std::make_shared<TIDTemporaryValue>(SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
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
    std::shared_ptr<TIDValue> value = Expression();
    provided.push_back(value->GetType());
    while (IsLexeme(LexemeType::kPunctuation, L",")) {
      GetNext();
      value = Expression();
      provided.push_back(value->GetType());
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

std::shared_ptr<TIDValue> Priority1() {
  std::vector<std::pair<BinaryOperator, std::shared_ptr<TIDValue>>> st;
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
    std::shared_ptr<TIDValue> rhs = st.back().second;
    BinaryOperator op = st.back().first;
    st.pop_back();
    std::shared_ptr<TIDValue> lhs = st.back().second;
    BinaryOperator next_op = st.back().first;
    st.pop_back();
    st.emplace_back(next_op, BinaryOperationRPN(lhs, op, rhs, lexeme, *rpn.back()));
  }
  return st[0].second;
}

std::shared_ptr<TIDValue> Priority2() {
  auto val = Priority3();
  while (IsLexeme(LexemeType::kOperator, L"||")) {
    GetNext();
    auto new_val = Priority3();
    val = BinaryOperationRPN(val, BinaryOperator::kLogicalOr, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority3() {
  auto val = Priority4();
  while (IsLexeme(LexemeType::kOperator, L"&&")) {
    GetNext();
    auto new_val = Priority4();
    val = BinaryOperationRPN(val, BinaryOperator::kLogicalAnd, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority4() {
  auto val = Priority5();
  while (IsLexeme(LexemeType::kOperator, L"|")) {
    GetNext();
    auto new_val = Priority5();
    val = BinaryOperationRPN(val, BinaryOperator::kBitwiseOr, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority5() {
  auto val = Priority6();
  while (IsLexeme(LexemeType::kOperator, L"^")) {
    GetNext();
    auto new_val = Priority6();
    val = BinaryOperationRPN(val, BinaryOperator::kBitwiseXor, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority6() {
  auto val = Priority7();
  while (IsLexeme(LexemeType::kOperator, L"&")) {
    GetNext();
    auto new_val = Priority7();
    val = BinaryOperationRPN(val, BinaryOperator::kBitwiseAnd, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority7() {
  auto val = Priority8();
  while (IsLexeme(LexemeType::kOperator, L"==")
      || IsLexeme(LexemeType::kOperator, L"!=")) {
    BinaryOperator op = IsLexeme(LexemeType::kOperator, L"==") ? BinaryOperator::kEqual : BinaryOperator::kNotEqual;
    GetNext();
    auto new_val = Priority8();
    val = BinaryOperationRPN(val, op, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority8() {
  auto val = Priority9();
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
    auto new_val = Priority9();
    val = BinaryOperationRPN(val, op, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority9() {
  auto val = Priority10();
  while (IsLexeme(LexemeType::kOperator, L"<<")
      || IsLexeme(LexemeType::kOperator, L">>")) {
    BinaryOperator op = IsLexeme(LexemeType::kOperator, L"<<") ? BinaryOperator::kBitwiseShiftLeft
        : BinaryOperator::kBitwiseShiftRight;
    GetNext();
    auto new_val = Priority10();
    val = BinaryOperationRPN(val, op, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority10() {
  auto val = Priority11();
  while (IsLexeme(LexemeType::kOperator, L"+")
      || IsLexeme(LexemeType::kOperator, L"-")) {
    BinaryOperator op = IsLexeme(LexemeType::kOperator, L"+") ? BinaryOperator::kAddition
        : BinaryOperator::kSubtraction;
    GetNext();
    auto new_val = Priority11();
    val = BinaryOperationRPN(val, op, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority11() {
  auto val = Priority12();
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
    auto new_val = Priority12();
    val = BinaryOperationRPN(val, op, new_val, lexeme, *rpn.back());
  }
  return val;
}

std::shared_ptr<TIDValue> Priority12() {
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
  auto val = Priority13();
  while (!ops.empty()) {
    val = UnaryPrefixOperationRPN(ops.back(), val, lexeme, *rpn.back());
    ops.pop_back();
  }
  return val;
}

std::shared_ptr<TIDValue> Priority13() {
  auto val = Priority14();
  auto type = val->GetType();
  while (true) {
    if (IsLexeme(LexemeType::kBracket, L"[")) {
      if (!type || type->GetType() != VariableType::kArray)
        throw TypeNotIndexed(lexeme);
      GetNext();
      auto index_val = Expression();
      Cast(index_val->GetType(), SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
      Expect(LexemeType::kBracket, L"]");
      GetNext();
      type = SetReferenceToType(std::static_pointer_cast<TIDArrayVariableType>(type)->GetValue(), true);
      val = std::make_shared<TIDTemporaryValue>(type);
      PushNode(RPNOperandNode(type->GetSize()));
      PushNode(RPNOperatorNode(RPNOperatorType::kMultiply, PrimitiveVariableType::kUint64));
      PushNode(RPNOperandNode(4));
      PushNode(RPNOperatorNode(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
      PushNode(RPNOperatorNode(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
      LoadType(type);
    } else if (IsLexeme(LexemeType::kParenthesis, L"(")) {
      if (!type || type->GetType() != VariableType::kFunction)
        throw TypeNotCallable(lexeme);
      std::wstring name;
      if (val->GetValueType() == TIDValueType::kVariable)
        name = std::static_pointer_cast<TIDVariable>(val)->GetName();
      if (name == L"new") {
        val = New();
      } else if (name == L"delete") {
        val = Delete();
      } else if (name == L"size") {
        val = Size();
      } else if (name == L"sizeof") {
        val = Sizeof();
      } else {
        FunctionCall(type);
        val = std::make_shared<TIDTemporaryValue>(std::static_pointer_cast<TIDFunctionVariableType>(type)->GetReturnType());
      }
      type = val->GetType();
    } else if (IsLexeme(LexemeType::kOperator, L".")) {
      if (!type || type->GetType() != VariableType::kComplex)
        throw TypeNoMembers(lexeme);
      GetNext();
      Expect(LexemeType::kIdentifier);
      std::wstring member_name = lexeme.GetValue();
      auto complex_type = std::dynamic_pointer_cast<TIDComplexVariableType>(type);
      auto new_type = complex_type->GetField(member_name);
      if (new_type == nullptr)
        throw TypeUnknownMember(lexeme);
      auto offset = complex_type->GetOffset(member_name);
      GetNext();
      type = SetParamsToType(new_type, type->IsConst(), true);
      val = std::make_shared<TIDTemporaryValue>(type);
      PushNode(RPNOperandNode(offset));
      PushNode(RPNOperatorNode(RPNOperatorType::kAdd));
      LoadType(type);
    } else if (IsLexeme(LexemeType::kOperator, L"++") || IsLexeme(LexemeType::kOperator, L"--")) {
      UnaryPostfixOperator op = IsLexeme(LexemeType::kOperator, L"++") ? UnaryPostfixOperator::kIncrement
        : UnaryPostfixOperator::kDecrement;
      GetNext();
      val = UnaryPostfixOperationRPN(val, op, lexeme, *rpn.back());
      type = val->GetType();
    } else if (IsLexeme(LexemeType::kReserved, L"as")) {
      GetNext();
      std::shared_ptr<TIDVariableType> new_type = Type(true);
      Cast(type, new_type, true);
      type = new_type;
      val = std::make_shared<TIDTemporaryValue>(type);
    } else
      break;
  }
  return val;
}

std::shared_ptr<TIDValue> Priority14() {
  if (IsLexeme(LexemeType::kParenthesis, L"(")) {
    GetNext();
    auto type = Expression();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    return type;
  } else {
    std::shared_ptr<TIDValue> val;
    if (IsLexeme(LexemeType::kNumericLiteral)) {
      PrimitiveVariableType primitive_type = NumericTypeFromString(lexeme.GetValue());
      val = std::make_shared<TIDTemporaryValue>(SetConstToType(
          GetPrimitiveVariableType(primitive_type), true));
      uint64_t value = (primitive_type == PrimitiveVariableType::kF32 ||
        primitive_type == PrimitiveVariableType::kF64) ? DecimalFromString(lexeme.GetValue(), primitive_type)
                                                       : IntegerFromString(lexeme.GetValue(), primitive_type);
        PushNode(RPNOperandNode(value));
    } else if (IsLexeme(LexemeType::kIdentifier)) {
      std::shared_ptr<TIDVariableType> struct_type = tid.GetComplexStruct(lexeme.GetValue());
      if (struct_type) {
        Expect(LexemeType::kParenthesis, L"(");
        GetNext();
        Expect(LexemeType::kParenthesis, L")");
        GetNext();
        val = std::make_shared<TIDTemporaryValue>(SetParamsToType(struct_type, true, false));
        uint64_t address = tid.AddTemporaryStructInstance(lexeme, val->GetType());
        PushNode(RPNOperandNode(address));
        PushNode(RPNOperatorNode(RPNOperatorType::kFromSP));
      } else {
        val = tid.GetVariable(lexeme.GetValue());
        if (!val)
          throw UndeclaredIdentifier(lexeme);
        auto type = val->GetType();
        PushNode(RPNOperandNode(std::dynamic_pointer_cast<TIDVariable>(val)->GetAddress()));
        PushNode(RPNOperatorNode(RPNOperatorType::kFromSP));
        LoadType(type);
      }
    } else if (IsLexeme(LexemeType::kStringLiteral)) {
      val = std::make_shared<TIDTemporaryValue>(
          SetConstToType(DeriveArrayFromType(GetPrimitiveVariableType(PrimitiveVariableType::kChar)), true)
      );
      PushNode(RPNOperandNode(lexeme.GetValue().size() + 4));
      PushNode(RPNOperatorNode(RPNOperatorType::kNew));
    } else if (IsLexeme(LexemeType::kCharLiteral)) {
      val = std::make_shared<TIDTemporaryValue>(
          SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kChar), true)
      );
      PushNode(RPNOperandNode(static_cast<unsigned char>(lexeme.GetValue()[0])));
    } else if (IsLexeme(LexemeType::kReserved, L"true") || IsLexeme(LexemeType::kReserved, L"false")) {
      val = std::make_shared<TIDTemporaryValue>(
          SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true)
      );
      PushNode(RPNOperandNode(IsLexeme(LexemeType::kReserved, L"true")));
    }
    if (!val)
      throw ExpectedExpression(lexeme);
    GetNext();
    return val;
  }
}
