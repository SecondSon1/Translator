#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <sstream>

enum class LexemeType : uint8_t {
  kReserved       = 0,
  kIdentifier     = 1,
  kNumericLiteral = 2,
  kOperator       = 3,
  kPunctuation    = 4,
  kParenthesis    = 5,
  kBracket        = 6,
  kStringLiteral  = 7,
  kCharLiteral    = 8,
  kVariableType   = 9,
  kUnknown        = 254,
  kNull           = 255
};

std::wstring ToString(LexemeType type);

class Lexeme {
 public:
  Lexeme() : type_(LexemeType::kUnknown), val_() {}
  Lexeme(const LexemeType & type, const std::wstring & value, size_t index) : type_(type), val_(value), index_(index) {}

  LexemeType GetType() const { return type_; }
  std::wstring GetValue() const { return val_; }
  size_t GetIndex() const { return index_; }

  friend std::wostream& operator << (std::wostream & out, const Lexeme & lexeme);

 private:
  LexemeType type_;
  std::wstring val_;
  size_t index_;
};

std::map<LexemeType, std::vector<std::wstring>> GetLexemeStrings();
