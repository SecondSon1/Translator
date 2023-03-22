#pragma once

enum class VariableType {
    kInt8, kInt16, kInt32, kInt64,
    kUint8, kUint16, kUint32, kUint64,
    kF32, kF64,
    kString,
    kBool,
    kVoid,
    kFunc
};

class Variable {
public:
    Variable(const VariableType type) : _type(type) {};

private:
    const VariableType _type;
};
