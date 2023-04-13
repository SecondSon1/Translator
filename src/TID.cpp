#include <map>

#include "TID.hpp"

void TID::CreateScope() {
  Node *q = new Node;
  current->scopes.push_back(q);
  q->parent = current;
  current = q;
}

void TID::ExitScope() {
  current = current->parent;
}

void TID::CreateVariable(const std::wstring & name) {
  current->variables.insert({name, 0});
}

bool TID::CheckVariable(const std::wstring & name) {
  Node *q = current;
  while (q != nullptr) {
    if (q->variables.find(name) != q->variables.end()) return true;
    q = q->parent;
  }
  return false;
}
