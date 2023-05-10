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

constexpr uint64_t NULLPTR = 0;

std::vector<std::shared_ptr<TIDVariableType>> scope_return_type;
uint32_t surrounding_loop_count = 0;

std::vector<Lexeme> _lexemes;
size_t _lexeme_index;
bool eof;
Lexeme lexeme;
TID tid;

void GetNext() {
  _lexeme_index++;
  if (_lexeme_index >= _lexemes.size()) {
    eof = true;
    lexeme = Lexeme(LexemeType::kNull, L"", lexeme.GetIndex() + lexeme.GetValue().size());
  } else
    lexeme = _lexemes[_lexeme_index];
}

void Program();

std::map<std::wstring, std::shared_ptr<RPN>> func_rpn;
std::map<std::wstring, uint64_t> func_size;
std::vector<std::shared_ptr<RPN>> rpn;

RPN PerformSyntaxAnalysis(const std::vector<Lexeme> & code) {
  if (code.empty()) return {};
  _lexemes = code;
  _lexeme_index = 0;
  lexeme = code[0];
  eof = false;
  scope_return_type.push_back(GetPrimitiveVariableType(PrimitiveVariableType::kInt32));
  rpn.push_back(std::make_shared<RPN>());
  Program();
  // In TID function scope is still open
  // This is size of global scope stack item
  uint64_t global_stack_size = tid.GetFunctionScopeMaxAddress();
  RPN result;
  result.PushNode(RPNOperand(global_stack_size));
  result.PushNode(RPNOperand(0));
  result.PushNode(RPNOperator(RPNOperatorType::kPush));
  result.PushNode(RPNOperand(-1ull));
  result.PushNode(RPNOperator(RPNOperatorType::kSP));
  result.PushNode(RPNOperator(RPNOperatorType::kStoreDA, PrimitiveVariableType::kUint64));
  for (std::shared_ptr<RPNNode> & node : rpn.back()->GetNodes()) {
    if (node->GetNodeType() == NodeType::kRelativeOperand) {
      uint64_t val = std::dynamic_pointer_cast<RPNRelativeOperand>(node)->GetValue();
      result.PushNode(std::make_shared<RPNOperand>(val + 6));
    } else
      result.PushNode(std::move(node));
  }
  AddReturn(result);
  std::map<std::wstring, uint64_t> pc_by_name;
  uint64_t pc = result.GetNodes().size();
  for (auto & [name, cur_rpn] : func_rpn) {
    pc_by_name[name] = pc;
    auto begin = pc;
    for (auto & node : cur_rpn->GetNodes()) {
      if (node->GetNodeType() == NodeType::kRelativeOperand) {
        uint64_t val = std::dynamic_pointer_cast<RPNRelativeOperand>(node)->GetValue();
        result.PushNode(RPNOperand(val + begin));
      } else
        result.PushNode(std::move(node));
      ++pc;
    }
    AddReturn(result);
  }
  for (auto & node : result.GetNodes()) {
    if (node->GetNodeType() == NodeType::kReferenceOperand) {
      std::wstring name = std::dynamic_pointer_cast<RPNReferenceOperand>(node)
        ->GetName();
      if (!pc_by_name.count(name))
        throw VariableNotFoundByInternalName();
      node = std::make_shared<RPNOperand>(pc_by_name[name]);
    }
  }
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
//std::vector<std::pair<std::wstring, std::pair<std::shared_ptr<TIDVariableType>, uint64_t>>> Definition();
//void DefinitionAddToScope();
uint64_t Definition();
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
std::shared_ptr<TIDValue> Read();
std::shared_ptr<TIDValue> Write();
void If();
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

void PushNode(const RPNOperand & node) { rpn.back()->PushNode(node); }
void PushNode(const RPNReferenceOperand & node) { rpn.back()->PushNode(node); }
void PushNode(const RPNOperator & node) { rpn.back()->PushNode(node); }
void PushNode(const RPNRelativeOperand & node) { rpn.back()->PushNode(node); }
void LoadType(const std::shared_ptr<TIDVariableType> & type, bool need_reference) {
  if (type->GetType() != VariableType::kComplex && !need_reference)
    PushNode(RPNOperator(RPNOperatorType::kLoad, GetTypeOfVariable(type)));
}

void ReplaceReferenceOperands(uint64_t l, uint64_t r, const std::wstring & from, uint64_t to) {
  auto & nodes = rpn.back()->GetNodes();
  for (uint64_t i = l; i < r; ++i) {
    if (nodes[i]->GetNodeType() == NodeType::kReferenceOperand &&
        std::dynamic_pointer_cast<RPNReferenceOperand>(nodes[i])->GetName() == from)
      nodes[i] = std::make_shared<RPNRelativeOperand>(to);
  }
}

void Cast(const std::shared_ptr<TIDValue> & from, const std::shared_ptr<TIDVariableType> & to, [[maybe_unused]] bool suppress_downcast_warning = false) {
  if (!from && !to) return;
  if (!from || !to || !CanCast(from, to))
    throw TypeMismatch(lexeme, to, from->GetType());

  if (!suppress_downcast_warning && !CanCastLossless(from, to))
    log::warning(Downcast(lexeme, from->GetType(), to));

  Cast(from, to, *rpn.back());
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
  bool reserved = IsLexeme(LexemeType::kReserved);
  if (IsLexeme(LexemeType::kReserved, L"new") || IsLexeme(LexemeType::kReserved, L"delete") ||
      IsLexeme(LexemeType::kReserved, L"size") || IsLexeme(LexemeType::kReserved, L"sizeof") ||
      IsLexeme(LexemeType::kReserved, L"read") || IsLexeme(LexemeType::kReserved, L"write"))
    reserved = false;
  if (reserved || IsLexeme(LexemeType::kVariableType) ||
      (IsLexeme(LexemeType::kIdentifier) && tid.GetComplexStruct(lexeme.GetValue()) != nullptr))
    Keyword();
  else if (IsLexeme(LexemeType::kPunctuation, L"{"))
    Block(true);
  else {
    std::shared_ptr<TIDValue> val;
    if (!IsLexeme(LexemeType::kPunctuation, L";"))
      val = Expression();
    if (val && val->GetType())
      PushNode(RPNOperator(RPNOperatorType::kDump));
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
    Definition();
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
  auto begin = tid.GetNextAddress();
  tid.AddScope();
  uint64_t size = 0;
  while (!IsLexeme(LexemeType::kPunctuation, L"}")) {
    size += Definition();
  }
  for (std::shared_ptr<TIDVariable> var : tid.GetLastScopeVariables())
    result.emplace_back(var->GetName(), var->GetType());
  std::shared_ptr<TIDVariableType> complex_type = std::make_shared<TIDComplexVariableType>(name, result);
  PushNode(RPNOperand(size));
  PushNode(RPNOperator(RPNOperatorType::kNew));
  PushNode(RPNOperator(RPNOperatorType::kDuplicate));
  PushNode(RPNOperand(begin));
  PushNode(RPNOperator(RPNOperatorType::kFromSP));
  PushNode(RPNOperand(size));
  PushNode(RPNOperator(RPNOperatorType::kCopyTF));
  tid.RemoveScope();
  tid.AddComplexStruct(lexeme, complex_type);
  auto complex_struct = tid.GetComplexStruct(name);
  assert(complex_struct);
  auto internal_name = std::dynamic_pointer_cast<TIDComplexVariableType>(complex_struct)->GetInternalName();

  // Add to tid pointer to that
  std::wstring def_var_name = internal_name + L"$def";
  tid.AddVariable(lexeme, def_var_name, SetParamsToType(DerivePointerFromType(complex_type), false, false));
  auto def_var = tid.GetVariable(def_var_name);
  assert(def_var);
  tid.LoadVariableAddress(def_var_name, *rpn.back());
  PushNode(RPNOperator(RPNOperatorType::kStoreDA, PrimitiveVariableType::kUint64));

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
    /*
    if (!IsLexeme(LexemeType::kBracket, L"]")) {
      std::shared_ptr<TIDValue> ind_val = Expression();
      Cast(ind_val, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
    }
    */
    Expect(LexemeType::kBracket, L"]");
    GetNext();
    ptr = DeriveArrayFromType(ptr);
  }
  return { name, ptr };
}

uint64_t Definition() {
  debug("Definition");
  uint64_t result = 0;
  std::shared_ptr<TIDVariableType> type = Type(true);
  std::shared_ptr<TIDVariableType> var_type;
  std::wstring name;
  if (IsLexeme(LexemeType::kIdentifier)) {
    name = lexeme.GetValue();
    GetNext();
    if (IsLexeme(LexemeType::kParenthesis)) {
      std::shared_ptr<TIDVariableType> func_type = Function(name, type);
      return 8;
    }

    if (type == nullptr)
      throw VoidNotExpected(lexeme);
    var_type = type;
    while (IsLexeme(LexemeType::kBracket, L"[")) {
      GetNext();
      /* std::shared_ptr<TIDValue> ind_val = Expression();
      Cast(ind_val, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true)); */
      Expect(LexemeType::kBracket, L"]");
      GetNext();
      var_type = DeriveArrayFromType(var_type);
    }
  } else {
    tie(name, var_type) = VariableIdentifier(type);
  }
  tid.AddVariable(lexeme, name, var_type);
  auto var = tid.GetVariable(name);
  assert(var);
  result += var_type->GetSize();

  auto Eq = [&var, &var_type]() {
    std::shared_ptr<TIDValue> expr_val = Expression();
    Cast(expr_val, SetConstToType(var_type, true));
    auto expr_type = expr_val->GetType();
    assert(expr_type);
    tid.LoadVariableAddress(var->GetName(), *rpn.back());
    if (expr_type->GetType() == VariableType::kComplex) {
      PushNode(RPNOperand(expr_type->GetSize()));
      PushNode(RPNOperator(RPNOperatorType::kCopyFT));
    } else {
      PushNode(RPNOperator(RPNOperatorType::kStoreDA, GetTypeOfVariable(var_type)));
    }
  };
  if (IsLexeme(LexemeType::kOperator, L"=")) {
    GetNext();
    Eq();
  }
  while (IsLexeme(LexemeType::kPunctuation, L",")) {
    GetNext();
    tie(name, var_type) = VariableIdentifier(type);
    tid.AddVariable(lexeme, name, var_type);
    var = tid.GetVariable(name);
    assert(var);
    result += var_type->GetSize();
    if (IsLexeme(LexemeType::kOperator, L"=")) {
      GetNext();
      Eq();
    }
  }
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  return result;
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
      Cast(param_val, SetConstToType(var.second, true));
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
  auto var = tid.GetVariable(name);
  assert(var);
  func_rpn[var->GetInternalName()] = rpn.back();

  if (!IsLexeme(LexemeType::kPunctuation, L";")) {
    tid.AddFunctionScope(var->GetInternalName(), return_type);
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
    AddReturn(*rpn.back());
    uint64_t stack_size = tid.GetFunctionScopeMaxAddress();
    func_size[tid.GetVariable(name)->GetInternalName()] = stack_size;
    tid.RemoveFunctionScope();
  } else {
    GetNext();
    PushNode(RPNOperator(RPNOperatorType::kReturn));
  }

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
  std::vector<uint64_t> to_set_end;
  std::shared_ptr<TIDValue> value = Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Cast(value, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
  uint64_t next_ind = rpn.back()->GetNodes().size();
  PushNode(RPNRelativeOperand(0));
  PushNode(RPNOperator(RPNOperatorType::kJz));
  Block(true);
  to_set_end.push_back(rpn.back()->GetNodes().size());
  PushNode(RPNRelativeOperand(0));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  while (IsLexeme(LexemeType::kReserved, L"elif")) {
    GetNext();
    Expect(LexemeType::kParenthesis, L"(");
    GetNext();
    rpn.back()->GetNodes()[next_ind] = std::make_shared<RPNRelativeOperand>(rpn.back()->GetNodes().size());
    value = Expression();
    Cast(value, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    next_ind = rpn.back()->GetNodes().size();
    PushNode(RPNOperand(0));
    PushNode(RPNOperator(RPNOperatorType::kJz));
    Block(true);
    to_set_end.push_back(rpn.back()->GetNodes().size());
    PushNode(RPNOperand(0));
    PushNode(RPNOperator(RPNOperatorType::kJmp));
  }
  rpn.back()->GetNodes()[next_ind] = std::make_shared<RPNRelativeOperand>(rpn.back()->GetNodes().size());
  if (IsLexeme(LexemeType::kReserved, L"else")) {
    Expect(LexemeType::kReserved, L"else");
    GetNext();
    Block(true);
  }
  uint64_t end = rpn.back()->GetNodes().size();
  for (uint64_t i : to_set_end)
    rpn.back()->GetNodes()[i] = std::make_shared<RPNRelativeOperand>(end);
}

void For() {
  debug("For");
  Expect(LexemeType::kReserved, L"for");
  GetNext();
  tid.AddScope();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  if (!IsLexeme(LexemeType::kPunctuation, L";")) {
    auto name = lexeme.GetValue();
    if (IsLexeme(LexemeType::kVariableType) || tid.GetComplexStruct(name))
      Definition();
    else {
      auto val = Expression();
      if (val->GetType())
        PushNode(RPNOperator(RPNOperatorType::kDump));
      GetNext();
    }
  } else
    GetNext();
  std::shared_ptr<TIDValue> val;
  uint64_t expression_start = rpn.back()->GetNodes().size();
  if (!IsLexeme(LexemeType::kPunctuation, L";")) {
    val = Expression();
    if (val)
      Cast(val, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
  }
  if (!val)
    PushNode(RPNOperand(1));
  uint64_t end_ind = rpn.back()->GetNodes().size();
  PushNode(RPNOperand(0));
  PushNode(RPNOperator(RPNOperatorType::kJz));
  uint64_t start_ind = rpn.back()->GetNodes().size();
  PushNode(RPNOperand(0));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  Expect(LexemeType::kPunctuation, L";");
  GetNext();
  uint64_t incr_start = rpn.back()->GetNodes().size();
  if (!IsLexeme(LexemeType::kParenthesis, L")")) {
    auto incr_val = Expression();
    if (incr_val->GetType())
      PushNode(RPNOperator(RPNOperatorType::kDump));
    while (IsLexeme(LexemeType::kPunctuation, L",")) {
      GetNext();
      incr_val = Expression();
      if (incr_val->GetType())
        PushNode(RPNOperator(RPNOperatorType::kDump));
    }
  }
  PushNode(RPNRelativeOperand(expression_start));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  uint64_t start = rpn.back()->GetNodes().size();

  ++surrounding_loop_count;
  Block(false);
  --surrounding_loop_count;
  PushNode(RPNRelativeOperand(incr_start));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  uint64_t end = rpn.back()->GetNodes().size();
  tid.RemoveScope();
  rpn.back()->GetNodes()[start_ind] = std::make_shared<RPNRelativeOperand>(start);
  rpn.back()->GetNodes()[end_ind] = std::make_shared<RPNRelativeOperand>(end);
  ReplaceReferenceOperands(start, end, L"continue", incr_start);
  ReplaceReferenceOperands(start, end, L"break", end);
  debug("Exited For");
}

// TODO
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
  auto derived_expected = GetDerivedTypes(std::make_shared<TIDTemporaryValue>(expected_type));
  bool ok = false;
  for (auto type : derived_expected)
    if (type->GetType() == iter_type) {
      ok = true;
      break;
    }
  if (!ok)
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
  uint64_t expression_start = rpn.back()->GetNodes().size();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  Cast(val, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
  uint64_t end_ind = rpn.back()->GetNodes().size();
  PushNode(RPNOperand(0));
  PushNode(RPNOperator(RPNOperatorType::kJz));
  GetNext();
  uint64_t start = rpn.back()->GetNodes().size();
  ++surrounding_loop_count;
  Block(true);
  --surrounding_loop_count;
  PushNode(RPNRelativeOperand(expression_start));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  uint64_t end = rpn.back()->GetNodes().size();
  rpn.back()->GetNodes()[end_ind] = std::make_shared<RPNRelativeOperand>(end);
  ReplaceReferenceOperands(start, end, L"continue", expression_start);
  ReplaceReferenceOperands(start, end, L"break", end);

}

void DoWhile() {
  Expect(LexemeType::kReserved, L"do");
  uint64_t start = rpn.back()->GetNodes().size();
  GetNext();
  Block(true);
  Expect(LexemeType::kReserved, L"while");
  GetNext();
  uint64_t expression_start = rpn.back()->GetNodes().size();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> value = Expression();
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  Cast(value, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true));
  uint64_t end_ind = rpn.back()->GetNodes().size();
  PushNode(RPNOperand(0));
  PushNode(RPNOperator(RPNOperatorType::kJz));
  PushNode(RPNRelativeOperand(start));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  uint64_t end = rpn.back()->GetNodes().size();
  rpn.back()->GetNodes()[end_ind] = std::make_shared<RPNRelativeOperand>(end);
  ReplaceReferenceOperands(start, end, L"continue", expression_start);
  ReplaceReferenceOperands(start, end, L"break", end);
  ++surrounding_loop_count;
  GetNext();
  --surrounding_loop_count;
}

void Continue() {
  Expect(LexemeType::kReserved, L"continue");
  if (surrounding_loop_count == 0)
    throw LoopInstructionsOutsideOfLoop(lexeme);
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  PushNode(RPNReferenceOperand(L"continue"));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  GetNext();
}

void Break() {
  Expect(LexemeType::kReserved, L"break");
  if (surrounding_loop_count == 0)
    throw LoopInstructionsOutsideOfLoop(lexeme);
  GetNext();
  Expect(LexemeType::kPunctuation, L";");
  PushNode(RPNReferenceOperand(L"break"));
  PushNode(RPNOperator(RPNOperatorType::kJmp));
  GetNext();
}

void Return() {
  Expect(LexemeType::kReserved, L"return");
  GetNext();
  std::shared_ptr<TIDValue> value;
  if (!IsLexeme(LexemeType::kPunctuation, L";"))
    value = Expression();
  Expect(LexemeType::kPunctuation, L";");
  Cast(value, SetConstToType(scope_return_type.back(), true));
  if (scope_return_type.back()) {
    PushNode(RPNOperand(8));
    PushNode(RPNOperator(RPNOperatorType::kFromSP));
    PushNode(RPNOperand(1));
    PushNode(RPNOperator(RPNOperatorType::kStoreAD, PrimitiveVariableType::kBool));

    PushNode(RPNOperand(9));
    PushNode(RPNOperator(RPNOperatorType::kFromSP));
    if (IsReference(scope_return_type.back())) {
      PushNode(RPNOperand(scope_return_type.back()->GetSize()));
      PushNode(RPNOperator(RPNOperatorType::kCopyFT));
    } else {
      PushNode(RPNOperator(RPNOperatorType::kStoreDA, GetTypeOfVariable(value->GetType())));
    }
  }
  PushNode(RPNOperator(RPNOperatorType::kReturn));
  GetNext();
}

std::shared_ptr<TIDValue> New() {
  Expect(LexemeType::kReserved, L"new");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDVariableType> type = Type(true);
  if (!type || (!IsLexeme(LexemeType::kParenthesis, L")") && !IsLexeme(LexemeType::kPunctuation, L",")))
    throw NewIncorrectUsage(lexeme, type);
  if (IsLexeme(LexemeType::kParenthesis, L")")) {
    GetNext();
    PushNode(RPNOperand(type->GetSize()));
    PushNode(RPNOperator(RPNOperatorType::kNew));
    return std::make_shared<TIDTemporaryValue>(SetConstToType(DerivePointerFromType(type), true));
  } else {
    Expect(LexemeType::kPunctuation, L",");
    GetNext();
    std::shared_ptr<TIDValue> ind = Expression();
    Expect(LexemeType::kParenthesis, L")");
    GetNext();
    Cast(ind, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
    PushNode(RPNOperator(RPNOperatorType::kDuplicate));
    PushNode(RPNOperator(RPNOperatorType::kSave));

    PushNode(RPNOperand(type->GetSize()));
    PushNode(RPNOperator(RPNOperatorType::kMultiply, PrimitiveVariableType::kUint64));
    PushNode(RPNOperand(4));
    PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
    PushNode(RPNOperator(RPNOperatorType::kNew));
    PushNode(RPNOperator(RPNOperatorType::kDuplicate));
    PushNode(RPNOperator(RPNOperatorType::kRestore));
    PushNode(RPNOperator(RPNOperatorType::kStoreAD, PrimitiveVariableType::kUint32));
    return std::make_shared<TIDTemporaryValue>(SetConstToType(DeriveArrayFromType(type), true));
  }
}

std::shared_ptr<TIDValue> Delete() {
  Expect(LexemeType::kReserved, L"delete");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  auto type = val->GetType();
  auto var_type = type->GetType();
  if (var_type != VariableType::kPointer && var_type != VariableType::kArray)
    throw DeleteIncorrectUsage(lexeme, val->GetType());
  GetNext();
  LoadIfReference(val, *rpn.back());
  std::shared_ptr<TIDVariableType> value_type;
  if (var_type == VariableType::kPointer) {
    value_type = std::dynamic_pointer_cast<TIDPointerVariableType>(type)->GetValue();
    PushNode(RPNOperand(value_type->GetSize()));
  } else {
    value_type = std::dynamic_pointer_cast<TIDPointerVariableType>(type)->GetValue();
    PushNode(RPNOperator(RPNOperatorType::kDuplicate));
    PushNode(RPNOperator(RPNOperatorType::kLoad, PrimitiveVariableType::kUint32));
    PushNode(RPNOperand(value_type->GetSize()));
    PushNode(RPNOperator(RPNOperatorType::kMultiply, PrimitiveVariableType::kUint64));
    PushNode(RPNOperand(4));
    PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
  }
  PushNode(RPNOperator(RPNOperatorType::kDelete));
  return std::make_shared<TIDTemporaryValue>(nullptr);
}

std::shared_ptr<TIDValue> Size() {
  Expect(LexemeType::kReserved, L"size");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  auto type = val->GetType();
  if (type->GetType() != VariableType::kArray)
    throw SizeIncorrectUsage(lexeme, val->GetType());
  GetNext();
  LoadIfReference(val, *rpn.back());
//  PushNode(RPNOperator(RPNOperatorType::kLoad, PrimitiveVariableType::kUint64));
  PushNode(RPNOperator(RPNOperatorType::kLoad, PrimitiveVariableType::kUint32));
  return std::make_shared<TIDTemporaryValue>(SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
}

std::shared_ptr<TIDValue> Sizeof() {
  Expect(LexemeType::kReserved, L"sizeof");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDVariableType> type = Type(true);
  if (!type)
    throw VoidNotExpected(lexeme);
  Expect(LexemeType::kParenthesis, L")");
  GetNext();
  PushNode(RPNOperand(type->GetSize()));
  return std::make_shared<TIDTemporaryValue>(SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
}

std::shared_ptr<TIDValue> Read() {
  Expect(LexemeType::kReserved, L"read");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  auto type = val->GetType();
  if (!type || type->IsConst() || (val->GetValueType() == TIDValueType::kTemporary && !type->IsReference())
      || SetParamsToType(type, false, false) != DeriveArrayFromType(GetPrimitiveVariableType(PrimitiveVariableType::kChar)))
    throw ReadIncorrectUsage(lexeme, type);
  GetNext();
  PushNode(RPNOperator(RPNOperatorType::kRead));
  return std::make_shared<TIDTemporaryValue>(nullptr);
}

std::shared_ptr<TIDValue> Write() {
  Expect(LexemeType::kReserved, L"write");
  GetNext();
  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::shared_ptr<TIDValue> val = Expression();
  Expect(LexemeType::kParenthesis, L")");
  auto type = val->GetType();
  if (!type || SetParamsToType(type, false, false) !=
               DeriveArrayFromType(GetPrimitiveVariableType(PrimitiveVariableType::kChar)))
    throw WriteIncorrectUsage(lexeme, type);
  GetNext();
  LoadIfReference(val, *rpn.back());
  PushNode(RPNOperator(RPNOperatorType::kWrite));
  return std::make_shared<TIDTemporaryValue>(nullptr);
}

void FunctionCall(const std::shared_ptr<TIDValue> & val) {
  debug("Function Call");
  auto type = val->GetType();
  if (type->GetType() != VariableType::kFunction)
    throw ExpectedFunction(type);
  std::shared_ptr<TIDFunctionVariableType> func_type = std::dynamic_pointer_cast<TIDFunctionVariableType>(type);
  const std::vector<std::shared_ptr<TIDVariableType>> & params = func_type->GetParameters();
  const std::vector<std::shared_ptr<TIDVariableType>> & default_params = func_type->GetDefaultParameters();

  Expect(LexemeType::kParenthesis, L"(");
  GetNext();
  std::vector<std::shared_ptr<TIDValue>> provided;
  if (!IsLexeme(LexemeType::kParenthesis, L")")) {
    std::shared_ptr<TIDValue> value = Expression();
    provided.push_back(value);
    while (IsLexeme(LexemeType::kPunctuation, L",")) {
      GetNext();
      value = Expression();
      provided.push_back(value);
    }
  }
  Expect(LexemeType::kParenthesis, L")");
  GetNext();

  size_t param_index = 0, default_param_index = 0;
  for (const std::shared_ptr<TIDValue> & provided_val : provided) {
    if (param_index == params.size()) {
      if (default_param_index == default_params.size() ||
          !CanCast(provided_val, SetConstToType(default_params[default_param_index++], true)))
        throw FunctionParameterListDoesNotMatch(lexeme, func_type, provided);
    } else {
      if (!CanCast(provided_val,
                   SetConstToType(params[param_index++], true)))
        throw FunctionParameterListDoesNotMatch(lexeme, func_type, provided);
    }
  }
  if (param_index < params.size())
    throw FunctionParameterListDoesNotMatch(lexeme, func_type, provided);

  PushNode(RPNOperand(func_size[std::dynamic_pointer_cast<TIDVariable>(val)->GetInternalName()]));
  PushNode(RPNOperator(RPNOperatorType::kPush));
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
      LoadIfReference(val, *rpn.back());
      auto index_val = Expression();
      Cast(index_val, SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kUint32), true));
      Expect(LexemeType::kBracket, L"]");
      GetNext();
      type = SetReferenceToType(std::static_pointer_cast<TIDArrayVariableType>(type)->GetValue(), true);
      val = std::make_shared<TIDTemporaryValue>(type);
      PushNode(RPNOperand(SetReferenceToType(type, false)->GetSize()));
      PushNode(RPNOperator(RPNOperatorType::kMultiply, PrimitiveVariableType::kUint64));
      PushNode(RPNOperand(4));
      PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
      PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
    } else if (IsLexeme(LexemeType::kParenthesis, L"(")) {
      if (!type || type->GetType() != VariableType::kFunction)
        throw TypeNotCallable(lexeme);
      std::wstring name;
      if (val->GetValueType() == TIDValueType::kVariable)
        name = std::static_pointer_cast<TIDVariable>(val)->GetName();
      FunctionCall(val);
      val = std::make_shared<TIDTemporaryValue>(std::static_pointer_cast<TIDFunctionVariableType>(type)->GetReturnType());
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
      PushNode(RPNOperand(offset));
      PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
    } else if (IsLexeme(LexemeType::kOperator, L"++") || IsLexeme(LexemeType::kOperator, L"--")) {
      UnaryPostfixOperator op = IsLexeme(LexemeType::kOperator, L"++") ? UnaryPostfixOperator::kIncrement
        : UnaryPostfixOperator::kDecrement;
      GetNext();
      val = UnaryPostfixOperationRPN(val, op, lexeme, *rpn.back());
      type = val->GetType();
    } else if (IsLexeme(LexemeType::kReserved, L"as")) {
      GetNext();
      std::shared_ptr<TIDVariableType> new_type = Type(true);
      Cast(val, new_type, true);
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
    if (IsLexeme(LexemeType::kReserved, L"new") || IsLexeme(LexemeType::kReserved, L"delete") ||
        IsLexeme(LexemeType::kReserved, L"size") || IsLexeme(LexemeType::kReserved, L"sizeof") ||
        IsLexeme(LexemeType::kReserved, L"read") || IsLexeme(LexemeType::kReserved, L"write")) {
      if (IsLexeme(LexemeType::kReserved, L"new")) val = New();
      else if (IsLexeme(LexemeType::kReserved, L"delete")) val = Delete();
      else if (IsLexeme(LexemeType::kReserved, L"size")) val = Size();
      else if (IsLexeme(LexemeType::kReserved, L"sizeof")) val = Sizeof();
      else if (IsLexeme(LexemeType::kReserved, L"read")) val = Read();
      else val = Write();
      return val;
    } else if (IsLexeme(LexemeType::kNumericLiteral)) {
      PrimitiveVariableType primitive_type = NumericTypeFromString(lexeme.GetValue());
      val = std::make_shared<TIDTemporaryValue>(SetConstToType(
          GetPrimitiveVariableType(primitive_type), true));
      uint64_t value = (primitive_type == PrimitiveVariableType::kF32 ||
        primitive_type == PrimitiveVariableType::kF64) ? DecimalFromString(lexeme.GetValue(), primitive_type)
                                                       : IntegerFromString(lexeme.GetValue(), primitive_type);
        PushNode(RPNOperand(value));
    } else if (IsLexeme(LexemeType::kIdentifier)) {
      if (lexeme.GetValue() == L"nullptr") {
        PushNode(RPNOperand(NULLPTR));
        return std::make_shared<TIDTemporaryValue>(SetConstToType(DerivePointerFromType(
                GetPrimitiveVariableType(PrimitiveVariableType::kUint64)), true));
      }
      std::shared_ptr<TIDVariableType> struct_type = tid.GetComplexStruct(lexeme.GetValue());
      if (struct_type) {
        GetNext();
        Expect(LexemeType::kParenthesis, L"(");
        GetNext();
        Expect(LexemeType::kParenthesis, L")");
        val = std::make_shared<TIDTemporaryValue>(SetParamsToType(struct_type, true, false));
        uint64_t address = tid.AddTemporaryInstance(lexeme, val->GetType());
        PushNode(RPNOperand(address));
        PushNode(RPNOperator(RPNOperatorType::kFromSP));
        PushNode(RPNOperator(RPNOperatorType::kDuplicate));
        std::wstring internal_name = std::dynamic_pointer_cast<TIDComplexVariableType>(struct_type)->GetInternalName();
        auto def_var = tid.GetVariable(internal_name + L"$def");
        assert(def_var && def_var->GetType());
        tid.LoadVariableAddress(internal_name + L"$def", *rpn.back());
        LoadIfReference(def_var, *rpn.back());
        PushNode(RPNOperand(struct_type->GetSize()));
        PushNode(RPNOperator(RPNOperatorType::kCopyTF));
      } else {
        auto var = tid.GetVariable(lexeme.GetValue());
        val = var;
        if (!val)
          throw UndeclaredIdentifier(lexeme);
        auto type = val->GetType();
        tid.LoadVariableAddress(var->GetName(), *rpn.back());
      }
    } else if (IsLexeme(LexemeType::kStringLiteral)) {
      auto char_arr = SetConstToType(DeriveArrayFromType(
            GetPrimitiveVariableType(PrimitiveVariableType::kChar)), true);
      val = std::make_shared<TIDTemporaryValue>(char_arr);
      PushNode(RPNOperand(lexeme.GetValue().size() + 4));
      PushNode(RPNOperator(RPNOperatorType::kNew));
      auto address = tid.AddTemporaryInstance(lexeme, char_arr);
      PushNode(RPNOperand(address));
      PushNode(RPNOperator(RPNOperatorType::kFromSP));
      PushNode(RPNOperator(RPNOperatorType::kStoreDA, PrimitiveVariableType::kUint64));

      PushNode(RPNOperand(address));
      PushNode(RPNOperator(RPNOperatorType::kFromSP));
      PushNode(RPNOperator(RPNOperatorType::kLoad, PrimitiveVariableType::kUint64));

      PushNode(RPNOperator(RPNOperatorType::kDuplicate));
      PushNode(RPNOperand(lexeme.GetValue().size()));
      PushNode(RPNOperator(RPNOperatorType::kStoreAD, PrimitiveVariableType::kUint32));

      for (uint64_t i = 0; i < lexeme.GetValue().size(); ++i) {
        wchar_t ch = lexeme.GetValue()[i];
        PushNode(RPNOperator(RPNOperatorType::kDuplicate));
        PushNode(RPNOperand(i + 4));
        PushNode(RPNOperator(RPNOperatorType::kAdd, PrimitiveVariableType::kUint64));
        PushNode(RPNOperand(static_cast<unsigned char>(ch)));
        PushNode(RPNOperator(RPNOperatorType::kStoreAD, PrimitiveVariableType::kChar));
      }
    } else if (IsLexeme(LexemeType::kCharLiteral)) {
      val = std::make_shared<TIDTemporaryValue>(
          SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kChar), true)
      );
      PushNode(RPNOperand(static_cast<unsigned char>(lexeme.GetValue()[0])));
    } else if (IsLexeme(LexemeType::kReserved, L"true") || IsLexeme(LexemeType::kReserved, L"false")) {
      val = std::make_shared<TIDTemporaryValue>(
          SetConstToType(GetPrimitiveVariableType(PrimitiveVariableType::kBool), true)
      );
      PushNode(RPNOperand(IsLexeme(LexemeType::kReserved, L"true")));
    }
    if (!val)
      throw ExpectedExpression(lexeme);
    GetNext();
    return val;
  }
}
