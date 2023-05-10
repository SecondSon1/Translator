#include "run.hpp"
#include "TID.hpp"
#include "exceptions.hpp"
#include "generation.hpp"
#include <map>
#include <memory>
#include <string>
#include <iostream>

#undef assert

void Assert(bool x, std::wstring x_expr, uint64_t pc) {
  if (!x) {
    std::wcout << "Assertion failed: " << x_expr << std::endl;
    std::wcout << "pc = " << pc << std::endl;

    exit(1);
  }
}
#define assert(x) Assert(x, L"" #x, run::pc)

namespace run {
  constexpr uint32_t
  STACK_SIZE = 1 * 1024 * 1024;
  constexpr uint32_t
  MAX_SIZE = 10 * 1024 * 1024;
  constexpr uint64_t NULLPTR = 0;

// if index < STACK_SIZE -> it is on stack
// else, heap
  uint8_t memory[MAX_SIZE];
  uint8_t allocated[MAX_SIZE / 8];

  bool IsByteAllocated(uint64_t index) {
    return index < MAX_SIZE && ((allocated[index / 8] >> (index % 8)) & 1);
  }

  void AllocateByte(uint64_t index) {
    if (index >= MAX_SIZE)
      throw MemoryOutOfBoundsError();
    allocated[index / 8] |= 1 << (index % 8);
    memory[index] = 0;
  }

  void DeallocateByte(uint64_t index) {
    if (index >= MAX_SIZE)
      throw MemoryOutOfBoundsError();
    allocated[index / 8] &= (1 << (index % 8)) ^ 255;
  }

// [from; to)
// returns true if all bytes are allocated
// false if not
  bool IsChunkAllocated(uint64_t from, uint64_t to) {
    bool ok = true;
    for (uint64_t i = from; i < to; ++i)
      ok &= IsByteAllocated(i);
    return ok;
  }

// returns true if none of the bytes are allocated
// false if some do
  bool IsChunkNotAllocated(uint64_t from, uint64_t to) {
    bool ok = true;
    for (uint64_t i = from; i < to; ++i)
      ok &= !IsByteAllocated(i);
    return ok;
  }

  void AllocateChunk(uint64_t from, uint64_t to) {
    for (uint64_t i = from; i < to; ++i)
      AllocateByte(i);
  }

  void DeallocateChunk(uint64_t from, uint64_t to) {
    for (uint64_t i = from; i < to; ++i)
      DeallocateByte(i);
  }

  std::map<uint64_t, std::vector<uint64_t>> func_sps;
  std::vector<uint64_t> stack;

  struct SPItem {
    uint64_t address = 0;
    uint64_t function_pc = 0;
    uint64_t stack_size = 0;

    SPItem() {}

    SPItem(uint64_t address, uint64_t function_pc, uint64_t stack_size)
        : address(address), function_pc(function_pc), stack_size(stack_size) {}
  };

  std::vector<SPItem> sp_stack;
  uint64_t sp = 1; // stack pointer
  uint64_t hp = STACK_SIZE; // heap pointer
  uint64_t pc = 0; // program counter
  uint64_t program_size;

  void Push(uint64_t data) { stack.push_back(data); }

  uint64_t Pop() {
    uint64_t data = stack.back();
    stack.pop_back();
    return data;
  }

  std::pair<uint64_t, uint64_t> PopBin() {
    uint64_t r = Pop();
    return {Pop(), r};
  }

  uint64_t NewMemory(uint64_t size) {
    assert(IsChunkNotAllocated(hp, hp + size));
    AllocateChunk(hp, hp + size);
    uint64_t ptr = hp;
    hp += size;
    return ptr;
  }

  void DeleteMemory(uint64_t address, uint64_t size) {
    assert(IsChunkAllocated(address, address + size));
    if (address < STACK_SIZE) assert(false); // trying to delete memory on stack
    DeallocateChunk(address, address + size);
  }

  void WriteMemory(uint64_t data, uint64_t address, uint8_t size = 8) {
    for (uint32_t i = 0; i < size; ++i) {
      if (!IsByteAllocated(address + i)) throw MemoryNotAllocated();
      memory[address + i] = data >> (i * 8) & 255;
    }
  }

  uint64_t ReadMemory(uint64_t address, uint8_t size = 8) {
    uint64_t result = 0;
    for (uint64_t i = 0; i < size; ++i) {
      if (!IsByteAllocated(address + i))
        throw MemoryNotAllocated();
      result |= static_cast<uint64_t>(memory[address + i]) << (i * 8);
    }
    return result;
  }
  void Load(uint8_t size) {
    uint64_t address = Pop();
    if (address == NULLPTR) throw NullptrAccessedException();
    Push(ReadMemory(address, size));
  }

  void StoreDA(uint8_t size) {
    auto [data, address] = PopBin();
    WriteMemory(data, address, size);
  }

  void StoreAD(uint8_t size) {
    auto [address, data] = PopBin();
    WriteMemory(data, address, size);
  }

  void Jump(uint64_t address) {
    if (address >= program_size)
      throw JumpOutsideOfProgram();
    pc = address - 1; // it will ++ in program
  }

  void Jmp() {
    Jump(Pop());
  }

  void Call() {
    Jmp();
    func_sps[pc].push_back(sp_stack.back().address);
  }

  void Jz() {
    auto [cond, address] = PopBin();
    if (cond == 0) Jump(address);
  }

  void PushStack() {
    auto [size, function_pc] = PopBin();
    sp_stack.emplace_back(sp, function_pc, size);
    sp += size;
  }

  void PopStack() {
    sp -= sp_stack.back().stack_size;
    func_sps[sp_stack.back().function_pc].pop_back();
    sp_stack.pop_back();
  }

  void SP() {
    Push(sp_stack.back().address);
  }

  void FromSP() {
    auto address = Pop();
    Push(address + sp_stack.back().address);
  }

  void New() {
    auto size = Pop();
    Push(NewMemory(size));
  }

  void Delete() {
    auto [address, size] = PopBin();
    DeleteMemory(address, size);
  }

  void Read() {
    auto var_address = Pop(); // address of char[] variable
    std::wstring line;
    std::getline(std::wcin, line);
    if (line.back() == L'\n') line.pop_back();

    auto new_address = NewMemory(line.size() + 4);
    WriteMemory(line.size(), new_address, 4);
    for (size_t i = 0; i < line.size(); ++i)
      memory[new_address + 4 + i] = static_cast<unsigned char>(line[i]);
    WriteMemory(new_address, var_address, 8);
  }

  void Write() {
    auto address = Pop(); // address of start of char[]
    auto size = ReadMemory(address, 4);
    std::wstring line;
    line.resize(size);
    for (size_t i = 0; i < line.size(); ++i)
      line[i] = memory[address + 4 + i];
    std::wcout << line;
  }

  void Return() {
    auto cur_sp = sp_stack.back().address;
    auto ret_ptr = ReadMemory(cur_sp, 8);
    if (ret_ptr == -1ull)
      pc = program_size;
    else
      Jump(ret_ptr);
  }

  void FuncSP() {
    auto func_pc = Pop();
    if (func_sps[func_pc].empty())
      throw FunctionNotCalled();
    Push(func_sps[func_pc].back());
  }

  void Dump() {
    Pop();
  }

  void Duplicate() {
    Push(stack.back());
  }

  uint64_t saved_element = 0;
  void Save() {
    saved_element = Pop();
  }

  void Restore() {
    Push(saved_element);
  }

  void Copy(uint64_t from, uint64_t to, uint64_t size) {
    for (size_t i = 0; i < size; ++i) {
      if (!IsByteAllocated(from + i) || !IsByteAllocated(to + i))
        throw MemoryNotAllocated();
      memory[to + i] = memory[from + i];
    }
  }

  void CopyFT() {
    auto size = Pop();
    auto [from, to] = PopBin();
    Copy(from, to, size);
  }

  void CopyTF() {
    auto size = Pop();
    auto [to, from] = PopBin();
    Copy(from, to, size);
  }

  void Fill() {
    auto [from, size] = PopBin();
    for (uint64_t i = 0; i < size; ++i) {
      if (!IsByteAllocated(from + i))
        throw MemoryNotAllocated();
      memory[from + i] = 0;
    }
  }

  uint64_t PruneNum(uint64_t data, PrimitiveVariableType type) {
    auto size = type == PrimitiveVariableType::kBool ? 1 : GetSizeOfPrimitive(type);
    if (size == 8) return data;
    return data & ((1ull << (size * 8)) - 1); // 1ull << 64 = 0; 0 - 1 = -1 = 2^64-1
  }

  void ToF64(PrimitiveVariableType type) {
    auto data = Pop();
    data = PruneNum(data, type);
    double result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
        result = static_cast<uint8_t>(data);
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<int8_t>(data);
        break;
      case PrimitiveVariableType::kChar:
        result = static_cast<unsigned char>(data);
        break;
      case PrimitiveVariableType::kUint16:
        result = static_cast<uint16_t>(data);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<int16_t>(data);
        break;
      case PrimitiveVariableType::kUint32:
        result = static_cast<uint32_t>(data);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<int32_t>(data);
        break;
      case PrimitiveVariableType::kUint64:
        result = static_cast<uint64_t>(data);
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<int64_t>(data);
        break;
      case PrimitiveVariableType::kF32: {
        float data_f = *reinterpret_cast<float*>(&data);
        result = static_cast<double>(data_f);
      }
        break;
      case PrimitiveVariableType::kF64: {
        result = *reinterpret_cast<double*>(&data);
      }
        break;
      case PrimitiveVariableType::kBool:
        result = data;
        break;
      default: assert(false);
    }
    Push(*reinterpret_cast<uint64_t*>(&result));
  }

  void FromF64(PrimitiveVariableType type) {
    auto data = Pop();
    double data_d = *reinterpret_cast<double*>(&data);
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
        result = static_cast<uint8_t>(data_d);
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<uint8_t>(static_cast<int8_t>(data_d));
        break;
      case PrimitiveVariableType::kChar:
        result = static_cast<uint8_t>(data_d);
        break;
      case PrimitiveVariableType::kUint16:
        result = static_cast<uint16_t>(data_d);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<uint16_t>(static_cast<int16_t>(data_d));
        break;
      case PrimitiveVariableType::kUint32:
        result = static_cast<uint32_t>(data_d);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<uint32_t>(static_cast<int32_t>(data_d));
        break;
      case PrimitiveVariableType::kUint64:
        result = static_cast<uint64_t>(data_d);
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<uint64_t>(static_cast<int64_t>(data_d));
        break;
      case PrimitiveVariableType::kF32: {
        float data_f = static_cast<float>(data_d);
        result = *reinterpret_cast<uint32_t*>(&data_f);
      }
        break;
      case PrimitiveVariableType::kF64: {
        result = data;
      }
        break;
      case PrimitiveVariableType::kBool:
        result = static_cast<bool>(data);
        break;
      default: assert(false);
    }
    Push(result);
  }

  void ToInt64(PrimitiveVariableType type) {
    auto data = Pop();
    data = PruneNum(data, type);
    int64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
        result = static_cast<uint8_t>(data);
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<int8_t>(data);
        break;
      case PrimitiveVariableType::kChar:
        result = static_cast<unsigned char>(data);
        break;
      case PrimitiveVariableType::kUint16:
        result = static_cast<uint16_t>(data);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<int16_t>(data);
        break;
      case PrimitiveVariableType::kUint32:
        result = static_cast<uint32_t>(data);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<int32_t>(data);
        break;
      case PrimitiveVariableType::kUint64:
        result = static_cast<int64_t>(static_cast<uint64_t>(data));
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<int64_t>(data);
        break;
      case PrimitiveVariableType::kF32: {
        float data_f = *reinterpret_cast<float*>(&data);
        result = static_cast<int64_t>(data_f);
      }
        break;
      case PrimitiveVariableType::kF64: {
        result = static_cast<int64_t>(*reinterpret_cast<double*>(&data));
      }
        break;
      case PrimitiveVariableType::kBool:
        result = static_cast<bool>(data);
        break;
      default: assert(false);
    }
    Push(static_cast<uint64_t>(result));
  }

  void ToBool() {
    auto data = Pop();
    Push(static_cast<bool>(data));
  }

  void Minus(PrimitiveVariableType type) {
    auto data = Pop();
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kInt8:
      case PrimitiveVariableType::kChar:
        result = static_cast<uint8_t>(-static_cast<int8_t>(data));
        break;
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kInt16:
        result = static_cast<uint16_t>(-static_cast<int16_t>(data));
        break;
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kInt32:
        result = static_cast<uint32_t>(-static_cast<int32_t>(data));
        break;
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kInt64:
        result = static_cast<uint64_t>(-static_cast<int64_t>(data));
        break;
      case PrimitiveVariableType::kF32: {
        float data_f = *reinterpret_cast<float*>(&data);
        float res = -data_f;
        result = *reinterpret_cast<uint32_t*>(&res);
      }
        break;
      case PrimitiveVariableType::kF64: {
        double data_f = *reinterpret_cast<double*>(&data);
        double res = -data_f;
        result = *reinterpret_cast<uint64_t*>(&res);
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void Tilda(PrimitiveVariableType type) {
    auto data = Pop();
    uint64_t result = 0;
    uint64_t mask = PruneNum(~(0ull), type);
    result = result & mask ^ mask;
    Push(result);
  }

  void Add(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kInt8:
      case PrimitiveVariableType::kInt16:
      case PrimitiveVariableType::kInt32:
      case PrimitiveVariableType::kChar:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kInt64:
        result = PruneNum(lhs + rhs, type);
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        float sum = lhs_f + rhs_f;
        result = *reinterpret_cast<uint32_t*>(&sum);
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        double sum = lhs_f + rhs_f;
        result = *reinterpret_cast<uint64_t*>(&sum);
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void Subtract(PrimitiveVariableType type) {
    Minus(type);
    Add(type);
  }

  void Multiply(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kInt8:
      case PrimitiveVariableType::kInt16:
      case PrimitiveVariableType::kInt32:
      case PrimitiveVariableType::kChar:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kInt64:
        result = PruneNum(lhs * rhs, type);
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        float mul = lhs_f * rhs_f;
        result = *reinterpret_cast<uint32_t*>(&mul);
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        double mul = lhs_f * rhs_f;
        result = *reinterpret_cast<uint64_t*>(&mul);
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void Divide(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    if (!rhs) throw DivisionByZero();
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kChar:
        result = PruneNum(lhs / rhs, type);
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<uint8_t>(static_cast<int8_t>(lhs) / static_cast<int8_t>(rhs));
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<uint16_t>(static_cast<int16_t>(lhs) / static_cast<int16_t>(rhs));
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<uint32_t>(static_cast<int32_t>(lhs) / static_cast<int32_t>(rhs));
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<uint64_t>(static_cast<int64_t>(lhs) / static_cast<int64_t>(rhs));
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        float div = lhs_f / rhs_f;
        result = *reinterpret_cast<uint32_t*>(&div);
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        double div = lhs_f / rhs_f;
        result = *reinterpret_cast<uint64_t*>(&div);
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void Modulus(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    if (!rhs) throw DivisionByZero();
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kChar:
        result = PruneNum(lhs % rhs, type);
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<uint8_t>(static_cast<int8_t>(lhs) % static_cast<int8_t>(rhs));
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<uint16_t>(static_cast<int16_t>(lhs) % static_cast<int16_t>(rhs));
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<uint32_t>(static_cast<int32_t>(lhs) % static_cast<int32_t>(rhs));
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<uint64_t>(static_cast<int64_t>(lhs) % static_cast<int64_t>(rhs));
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        float div = std::fmodf(lhs_f, rhs_f);
        result = *reinterpret_cast<uint32_t*>(&div);
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        double div = std::fmod(lhs_f, rhs_f);
        result = *reinterpret_cast<uint64_t*>(&div);
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void BitwiseShiftLeft(PrimitiveVariableType type) {
    auto [val, sh] = PopBin();
    val <<= sh;
    Push(PruneNum(val, type));
  }

  void BitwiseShiftRight(PrimitiveVariableType type) {
    auto [val, sh] = PopBin();
    val >>= sh;
    Push(PruneNum(val, type));
  }

  void BitwiseAnd(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    Push(PruneNum(lhs & rhs, type));
  }

  void BitwiseOr(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    Push(PruneNum(lhs | rhs, type));
  }

  void BitwiseXor(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    Push(PruneNum(lhs ^ rhs, type));
  }

  void Invert(PrimitiveVariableType type) {
    auto data = Pop();
    Push(!PruneNum(data, type));
  }

  void Less(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    lhs = PruneNum(lhs, type);
    rhs = PruneNum(rhs, type);
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kChar:
        result = lhs < rhs;
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<int8_t>(lhs) < static_cast<int8_t>(rhs);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<int16_t>(lhs) < static_cast<int16_t>(rhs);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<int32_t>(lhs) < static_cast<int32_t>(rhs);
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<int64_t>(lhs) < static_cast<int64_t>(rhs);
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        result = lhs_f < rhs_f;
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        result = lhs_f < rhs_f;
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void More(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    lhs = PruneNum(lhs, type);
    rhs = PruneNum(rhs, type);
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kChar:
        result = lhs > rhs;
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<int8_t>(lhs) > static_cast<int8_t>(rhs);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<int16_t>(lhs) > static_cast<int16_t>(rhs);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<int32_t>(lhs) > static_cast<int32_t>(rhs);
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<int64_t>(lhs) > static_cast<int64_t>(rhs);
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        result = lhs_f > rhs_f;
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        result = lhs_f > rhs_f;
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void LessOrEqual(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    lhs = PruneNum(lhs, type);
    rhs = PruneNum(rhs, type);
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kChar:
        result = lhs <= rhs;
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<int8_t>(lhs) <= static_cast<int8_t>(rhs);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<int16_t>(lhs) <= static_cast<int16_t>(rhs);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<int32_t>(lhs) <= static_cast<int32_t>(rhs);
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<int64_t>(lhs) <= static_cast<int64_t>(rhs);
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        result = lhs_f <= rhs_f;
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        result = lhs_f <= rhs_f;
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void MoreOrEqual(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    lhs = PruneNum(lhs, type);
    rhs = PruneNum(rhs, type);
    uint64_t result = 0;
    switch (type) {
      case PrimitiveVariableType::kUint8:
      case PrimitiveVariableType::kUint16:
      case PrimitiveVariableType::kUint32:
      case PrimitiveVariableType::kUint64:
      case PrimitiveVariableType::kChar:
        result = lhs >= rhs;
        break;
      case PrimitiveVariableType::kInt8:
        result = static_cast<int8_t>(lhs) >= static_cast<int8_t>(rhs);
        break;
      case PrimitiveVariableType::kInt16:
        result = static_cast<int16_t>(lhs) >= static_cast<int16_t>(rhs);
        break;
      case PrimitiveVariableType::kInt32:
        result = static_cast<int32_t>(lhs) >= static_cast<int32_t>(rhs);
        break;
      case PrimitiveVariableType::kInt64:
        result = static_cast<int64_t>(lhs) >= static_cast<int64_t>(rhs);
        break;
      case PrimitiveVariableType::kF32: {
        float lhs_f = *reinterpret_cast<float*>(&lhs), rhs_f = *reinterpret_cast<float*>(&rhs);
        result = lhs_f >= rhs_f;
      }
        break;
      case PrimitiveVariableType::kF64: {
        double lhs_f = *reinterpret_cast<double*>(&lhs), rhs_f = *reinterpret_cast<double*>(&rhs);
        result = lhs_f >= rhs_f;
      }
        break;
      default: assert(false);
    }
    Push(result);
  }

  void Equal(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    Push(PruneNum(lhs, type) == PruneNum(rhs, type));
  }

  void NotEqual(PrimitiveVariableType type) {
    auto [lhs, rhs] = PopBin();
    Push(PruneNum(lhs, type) != PruneNum(rhs, type));
  }

  void HandleOperation(RPNOperatorType op, PrimitiveVariableType type) {
    switch (op) {
      // Internal operators
      case RPNOperatorType::kLoad:
        Load(static_cast<uint8_t>(GetSizeOfPrimitive(type)));
        break;
      case RPNOperatorType::kStoreDA:
        StoreDA(static_cast<uint8_t>(GetSizeOfPrimitive(type)));
        break;
      case RPNOperatorType::kStoreAD:
        StoreAD(static_cast<uint8_t>(GetSizeOfPrimitive(type)));
        break;
      case RPNOperatorType::kJmp:
        Jmp();
        break;
      case RPNOperatorType::kCall:
        Call();
        break;
      case RPNOperatorType::kJz:
        Jz();
        break;
      case RPNOperatorType::kPush:
        PushStack();
        break;
      case RPNOperatorType::kPop:
        PopStack();
        break;
      case RPNOperatorType::kSP:
        SP();
        break;
      case RPNOperatorType::kFromSP:
        FromSP();
        break;
      case RPNOperatorType::kNew:
        New();
        break;
      case RPNOperatorType::kDelete:
        Delete();
        break;
      case RPNOperatorType::kRead:
        Read();
        break;
      case RPNOperatorType::kWrite:
        Write();
        break;
      case RPNOperatorType::kReturn:
        Return();
        break;
      case RPNOperatorType::kFuncSP:
        FuncSP();
        break;
      case RPNOperatorType::kDump:
        Dump();
        break;
      case RPNOperatorType::kDuplicate:
        Duplicate();
        break;
      case RPNOperatorType::kSave:
        Save();
        break;
      case RPNOperatorType::kRestore:
        Restore();
        break;
      case RPNOperatorType::kCopyFT:
        CopyFT();
        break;
      case RPNOperatorType::kCopyTF:
        CopyTF();
        break;
      case RPNOperatorType::kFill:
        Fill();
        break;

        // Casting operators
      case RPNOperatorType::kToF64:
        ToF64(type);
        break;
      case RPNOperatorType::kFromF64:
        FromF64(type);
        break;
      case RPNOperatorType::kToBool:
        ToBool();
        break;
      case RPNOperatorType::kToInt64:
        ToInt64(type);
        break;

        // Arithmetic operators
      case RPNOperatorType::kMinus:
        Minus(type);
        break;
      case RPNOperatorType::kTilda:
        Tilda(type);
        break;
      case RPNOperatorType::kAdd:
        Add(type);
        break;
      case RPNOperatorType::kSubtract:
        Subtract(type);
        break;
      case RPNOperatorType::kMultiply:
        Multiply(type);
        break;
      case RPNOperatorType::kDivide:
        Divide(type);
        break;
      case RPNOperatorType::kModulus:
        Modulus(type);
        break;
      case RPNOperatorType::kBitwiseShiftLeft:
        BitwiseShiftLeft(type);
        break;
      case RPNOperatorType::kBitwiseShiftRight:
        BitwiseShiftLeft(type);
        break;
      case RPNOperatorType::kBitwiseAnd:
        BitwiseAnd(type);
        break;
      case RPNOperatorType::kBitwiseOr:
        BitwiseOr(type);
        break;
      case RPNOperatorType::kBitwiseXor:
        BitwiseXor(type);
        break;

        // Logical operators
      case RPNOperatorType::kInvert:
        Invert(type);
        break;
      case RPNOperatorType::kLess:
        Less(type);
        break;
      case RPNOperatorType::kMore:
        More(type);
        break;
      case RPNOperatorType::kLessOrEqual:
        LessOrEqual(type);
        break;
      case RPNOperatorType::kMoreOrEqual:
        MoreOrEqual(type);
        break;
      case RPNOperatorType::kEqual:
        Equal(type);
        break;
      case RPNOperatorType::kNotEqual:
        NotEqual(type);
        break;
    }
  }

}

int32_t Execute(const RPN & rpn_obj) {
  run::AllocateChunk(0, run::STACK_SIZE);
  run::func_sps[0].push_back(0);
  const std::vector<std::shared_ptr<RPNNode>> & rpn = rpn_obj.GetNodes();
  run::program_size = rpn.size();
  for (run::pc = 0; run::pc < rpn.size(); ++run::pc) {
    auto node = rpn[run::pc];
    if (node->GetNodeType() == NodeType::kReferenceOperand)
      throw ReferenceOperandMetError();
    if (node->GetNodeType() == NodeType::kOperand)
      run::stack.push_back(std::dynamic_pointer_cast<RPNOperand>(node)->GetValue());
    else {
      auto ptr = std::dynamic_pointer_cast<RPNOperator>(node);
      run::HandleOperation(ptr->GetOperatorType(), ptr->GetVariableType());
    }
  }

  int32_t return_code = 0;
  if (run::memory[9])
    return_code = static_cast<int32_t>(static_cast<uint32_t>(run::ReadMemory(10, 4)));
  return return_code;
}
