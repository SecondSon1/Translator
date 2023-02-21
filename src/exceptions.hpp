#include <stdexcept>
#include <string>

// ===============================
// === Lexical analysis errors ===
// ===============================

class LexicalAnalysisException : public std::exception {
 public:
  const char* what() const noexcept override {
    return "Unknown error at lexical analysis stage";
  }
};

class StringNotEndedException : public LexicalAnalysisException {
 public:
  StringNotEndedException(size_t index) : index_(index) {}

  size_t GetIndex() const { return index_; }

  const char* what() const noexcept override {
    return "String literal not ended";
  }

 private:
  size_t index_;
};

class UnknownEscapeSequenceException : public LexicalAnalysisException {
 public:
  UnknownEscapeSequenceException(size_t index) : index_(index) {}

  size_t GetIndex() const { return index_; }

  const char* what() const noexcept override {
    return "Unknown escape sequence exception";
  }

 private:
  size_t index_;
};
