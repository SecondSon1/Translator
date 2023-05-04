#include "lexeme.hpp"

std::map<LexemeType, std::vector<std::wstring>> GetLexemeStrings() {
  std::map<LexemeType, std::vector<std::wstring>> ans;
  ans[LexemeType::kReserved] = {
    L"for", L"while", L"do", L"foreach", L"of", L"if", L"elif", L"else", L"return", L"break", L"continue",
    L"struct", L"const", L"true", L"false", L"as"
  };
  ans[LexemeType::kOperator] = {
    L"+", L"-", L"*", L"/", L"%", L"++", L"--", L"&", L"|", L"^", L"&&", L"||", L"==", L"!=",
    L"<", L">", L"<=", L">=", L"<<", L">>", L"=", L"+=", L"-=", L"*=", L"/=", L"%=",
    L"<<=", L">>=", L"&=", L"^=", L"|=", L".", L"**", L"**=", L"~"
  };
  ans[LexemeType::kPunctuation] = {
    L",", L";", L"{", L"}", L"=>"
  };
  ans[LexemeType::kParenthesis] = {
    L"(", L")"
  };
  ans[LexemeType::kBracket] = {
    L"[", L"]"
  };
  ans[LexemeType::kVariableType] = {
    L"int8", L"int16", L"int32", L"int64", L"uint8", L"uint16", L"uint32", L"uint64", L"f32", L"f64", /*L"string",*/
    /*L"var", */L"void", L"bool", /*L"func", L"auto",*/ L"char"
  };
  return ans;
}


std::wstring ToString(LexemeType type) {
  switch (type) {
#define LexemeTypeCase(name) case LexemeType::name:\
    return L""#name
    LexemeTypeCase(kReserved);
    LexemeTypeCase(kIdentifier);
    LexemeTypeCase(kNumericLiteral);
    LexemeTypeCase(kOperator);
    LexemeTypeCase(kPunctuation);
    LexemeTypeCase(kParenthesis);
    LexemeTypeCase(kBracket);
    LexemeTypeCase(kStringLiteral);
    LexemeTypeCase(kVariableType);
    LexemeTypeCase(kUnknown);
    LexemeTypeCase(kNull);
#undef LexemeTypeCase
  }
}

std::wostream& operator << (std::wostream & out, const Lexeme & lexeme) {
  out << ToString(lexeme.type_) << ": " << lexeme.val_;
  return out;
}
