#include "io.hpp"

#include <fstream>
#include <string>


bool readFile(const std::string & fileName, std::wstring & text) {
  std::wifstream file(fileName);
  if (!file.is_open()) return false;

  std::wstring line;
  while (std::getline(file, line)) {
    text += line;
    text.push_back(L'\n');
  }
  file.close();

  return true;
}

template <typename E>
typename std::underlying_type<E>::type to_underlying(E e) {
  return static_cast<typename std::underlying_type<E>::type>(e);
}

bool saveProgram(const std::string & fileName, const RPN & rpn) {
  std::ofstream file(fileName, std::ios::binary);
  if (!file.is_open()) return false;

  // Buffer byte because c++
  uint8_t byte = 0;
  file.write((char *)&byte, sizeof(char)); // Version

  for (auto node : rpn.GetNodes()) {
    // Writing node type
    byte = to_underlying(node->GetNodeType());
    file.write((char *)&byte, sizeof(char));

    if (node->GetNodeType() == NodeType::kOperand) {
      const RPNOperand casted = dynamic_cast<const RPNOperand &>(*node);

      // Writing 8 bytes of value
      for (int8_t i = 56; i >= 0; i -= 8) {
        byte = (casted.GetValue() >> i) & 255;
        file.write((char *)&byte, sizeof(char));
      }
    }
    else if (node->GetNodeType() == NodeType::kOperator) {
      const RPNOperator casted = dynamic_cast<const RPNOperator &>(*node);

      // Writing 1 byte of operator
      byte = to_underlying(casted.GetOperatorType());
      file.write((char *)&byte, sizeof(char));

      // Writing 1 byte of variable
      byte = to_underlying(casted.GetVariableType());
      file.write((char *)&byte, sizeof(char));
    }
    else {
      // Something wrong, but I'm lazy
    }
  }
  file.close();

  return true;
}

bool readProgram(const std::string & fileName, RPN & rpn) {
  std::ifstream file(fileName, std::ios::binary);
  if (!file.is_open()) return false;

  uint8_t byte;
  file.read((char *)&byte, sizeof(char));

  if (byte > 0) throw "Unsupported file version";

  while (!file.eof()) {
    file.read((char *)&byte, sizeof(char));

    if (static_cast<NodeType>((uint8_t)byte) == NodeType::kOperand) {
      uint64_t value = 0;

      // Reading 8 bytes of value
      for (int8_t j = 0; j < 8; ++j) {
        file.read((char *)&byte, sizeof(char));
        value = (value << 8) | byte;
      }

      rpn.PushNode(RPNOperand(value));
    }
    else if (static_cast<NodeType>((uint8_t)byte) == NodeType::kOperator) {
      uint8_t op = 0, type = 0;

      // Reading 1 byte of operator
      file.read((char *)&op, sizeof(char));

      // Reading 1 byte of type
      file.read((char *)&type, sizeof(char));

      rpn.PushNode(RPNOperator(static_cast<RPNOperatorType>(op), static_cast<PrimitiveVariableType>(type)));
    }
    else {
      // Something wrong, but I'm lazy
    }
  }

  return true;
}
