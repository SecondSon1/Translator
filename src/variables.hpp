#pragma once

enum class VariableType {
    kInt8, kInt16, kInt32, kInt64,
    kUint8, kUint16, kUint32, kUint64,
    kF32, kF64,
    kString,
    kBool,
    kVoid,
    kFunction,
    kDynamic,
    kAuto
};

class Variable {
public:
    Variable(const VariableType type) : type_(type), returnType_(type) {};

private:
    VariableType type_, returnType_;
};
