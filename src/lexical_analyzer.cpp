#include "lexical_analyzer.hpp"
#include "exceptions.hpp"
#include "lexeme.hpp"

#include <iostream>

bool Fits(const std::wstring & code, size_t index, const std::wstring & token) {
  if (index + token.size() > code.size())
    return false;
  for (size_t i = 0; i < token.size(); ++i)
    if (code[index + i] != token[i])
      return false;
  return true;
}

bool IsDigit(wchar_t ch, bool is_hex) {
  if (std::isdigit(ch))
    return true;
  if (!is_hex)
    return false;
  return (L'a' <= ch && ch <= L'f') || (L'A' <= ch && ch <= L'F');
}

std::vector<Lexeme> PerformLexicalAnalysis(const std::wstring & code) {

  static std::vector<std::pair<std::wstring, LexemeType>> identifier_overrides;
  static std::vector<std::vector<std::pair<std::wstring, LexemeType>>> by_length;

  if (by_length.empty()) {

    std::map<LexemeType, std::vector<std::wstring>> lexeme_strings = GetLexemeStrings();
    for (const auto & [type, vec] : lexeme_strings) {
      if (type == LexemeType::kReserved || type == LexemeType::kVariableType) {
        for (const std::wstring & str : vec)
          identifier_overrides.emplace_back(str, type);
        continue;
      }
      for (const std::wstring & str : vec) {
        while (by_length.size() <= str.size())
          by_length.emplace_back();
        by_length[str.size()].emplace_back(str, type);
      }
    }

  }

  std::map<wchar_t, std::wstring> backslash_chars;
  backslash_chars[L'n'] = L"\n";
  backslash_chars[L'r'] = L"\r";
  backslash_chars[L't'] = L"\t";
  backslash_chars[L'\\'] = L"\\";
  backslash_chars[L'\n'] = L"";
  backslash_chars[L'"'] = L"\"";
  backslash_chars[L'\''] = L"'";

  std::vector<Lexeme> result;

  for (size_t i = 0, j = 1; i < code.size(); i = j++) {
    if (code[i] == L' ' || code[i] == L'\n' || code[i] == L'\t')
      continue;

    if (Fits(code, i, L"/*")) {                                   // multi-line comment
      while (j + 1 < code.size() && !Fits(code, j, L"*/"))
        ++j;
      if (!Fits(code, j, L"*/"))
        throw CommentNotEndedError(j);
      j += 2;
      continue;
    }

    if (Fits(code, i, L"//")) {                                   // comment
      while (j < code.size() && code[j] != '\n')
        ++j;
      continue;
    }

    std::wstring str;
    str.push_back(code[i]);

    if (std::isalpha(code[i]) || code[i] == '_') {                // identifier/reserved
      while (j < code.size() && (std::isalpha(code[j]) || code[j] == L'_' || std::isdigit(code[j]))) {
        str.push_back(code[j++]);
      }
      LexemeType resulting_type = LexemeType::kIdentifier;
      for (const std::pair<std::wstring, LexemeType> & ovd : identifier_overrides) {
        if (str == ovd.first) {
          resulting_type = ovd.second;
          break;
        }
      }
      result.emplace_back(resulting_type, str, i);
    } else if (std::isdigit(code[i])) {                           // numeric literal
      bool is_hex = false;
      bool is_decimal = false;
      if (Fits(code, i, L"0x")) {
        is_hex = true;
        ++j;
        str.push_back(L'x');
      }
      while (j < code.size() && IsDigit(code[j], is_hex)) {
        str.push_back(code[j++]);
      }
      if (j < code.size() && !is_hex && code[j] == '.') {
        str.push_back(code[j++]);
        if (j == code.size() || !std::isdigit(code[j]))
          throw NumberNotFinishedError(j);
        while (j < code.size() && std::isdigit(code[j]))
          str.push_back(code[j++]);
        is_decimal = true;
      }
      if (j < code.size()) {
        if (is_decimal) {
          if (code[j] == 'f' || code[j] == 'F')
            str.push_back(code[j++]);
        } else {
          if (code[j] == 'u' || code[j] == 'U')
            str.push_back(code[j++]);
          if (j < code.size() &&
              (code[j] == 'i' || code[j] == 'I' || code[j] == 'l' || code[j] == 'L' || code[j] == 's'
                || code[j] == 'S' || code[j] == 't' || code[j] == 'T'))
            str.push_back(code[j++]);
        }
      }
      result.emplace_back(LexemeType::kNumericLiteral, str, i);
    } else if (code[i] == L'"' || code[i] == L'\'') {
      wchar_t quote = code[i];
      str.pop_back();

      while (j < code.size() && code[j] != quote && code[j] != L'\n') {
        if (code[j] == L'\\') {
          if (j + 1 == code.size() || !backslash_chars.count(code[j+1]))
            throw UnknownEscapeSequenceError(j + 1);
          str += backslash_chars[code[j+1]];
          j += 2;
        } else
          str.push_back(code[j++]);
      }

      if (j >= code.size() || code[j] == L'\n')
        throw StringNotEndedError(j);
      result.emplace_back(LexemeType::kStringLiteral, str, i);
      ++j;
    } else {

      bool found = false;
      for (size_t len = std::min(by_length.size() - 1, code.size() - i); len > 0; --len) {
        for (const auto & [word, type] : by_length[len]) {
          if (Fits(code, i, word)) {
            found = true;
            result.emplace_back(type, word, i);
            j = i + len;
            break;
          }
        }
        if (found) break;
      }
      if (!found) {
        result.emplace_back(LexemeType::kUnknown, str, i);
      }

    }
  }
  return result;
}
