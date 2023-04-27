#include "casts.hpp"

#include <queue>

bool set_up_casting = false;

std::vector<PrimitiveVariableType> up_cast[kPrimitiveVariableTypeCount];
uint8_t layer[kPrimitiveVariableTypeCount];
// -1 - can't cast
//  0 - can cast without losses
//  1 - can cast but with losses
int8_t can_cast[kPrimitiveVariableTypeCount][kPrimitiveVariableTypeCount];

void SetUpCastingPrimitives() {
  for (uint8_t i = 0; i < kPrimitiveVariableTypeCount; ++i)
    std::fill(can_cast[i], can_cast[i] + kPrimitiveVariableTypeCount, -1);

  std::vector<uint8_t> rg[kPrimitiveVariableTypeCount];
#define Cast(x, y) up_cast[static_cast<uint8_t>(PrimitiveVariableType::x)].push_back(PrimitiveVariableType::y);\
  rg[static_cast<uint8_t>(PrimitiveVariableType::y)].push_back(static_cast<uint8_t>(PrimitiveVariableType::x))

  Cast(kBool, kInt8);
  Cast(kChar, kUint8);

  Cast(kInt8, kUint8);
  Cast(kInt8, kInt16);
  Cast(kUint8, kUint16);

  Cast(kInt16, kUint16);
  Cast(kInt16, kInt32);
  Cast(kUint16, kUint32);

  Cast(kInt32, kUint32);
  Cast(kInt32, kInt64);
  Cast(kUint32, kUint64);
  Cast(kInt32, kF32);
  Cast(kUint32, kF32);
  Cast(kF32, kF64);

  Cast(kInt64, kUint64);
  Cast(kInt64, kF64);
  Cast(kUint64, kF64);

  std::fill(layer, layer + kPrimitiveVariableTypeCount, -1);
  layer[static_cast<uint8_t>(PrimitiveVariableType::kF64)] = 0;
  std::queue<uint8_t> q;
  q.push(static_cast<uint8_t>(PrimitiveVariableType::kF64));
  while (!q.empty()) {
    uint8_t v = q.front();
    q.pop();
    for (uint8_t u : rg[v]) {
      if (layer[u] > layer[v] + 1) {
        layer[u] = layer[v] + 1;
        q.push(u);
      }
    }
  }

  for (uint8_t x = 0; x < kPrimitiveVariableTypeCount; ++x) {
    for (PrimitiveVariableType type : up_cast[x]) {
      can_cast[x][static_cast<uint8_t>(type)] = 0;
      can_cast[x][static_cast<uint8_t>(type)] = 1;
    }
  }
  for (uint8_t through = 0; through < kPrimitiveVariableTypeCount; ++through) {
    for (uint8_t a = 0; a < kPrimitiveVariableTypeCount; ++a) {
      for (uint8_t b = 0; b < kPrimitiveVariableTypeCount; ++b) {
        if (can_cast[a][through] != -1 && can_cast[through][b] != -1) {
          int8_t x = can_cast[a][through] || can_cast[through][b];
          if (can_cast[a][b] == -1 || x < can_cast[a][b])
            can_cast[a][b] = x;
        }
      }
    }
  }

  set_up_casting = true;
}

int8_t CastValue(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  if (from == to) return 0;
  if (from->GetType() != to->GetType()) return -1;
  if (from->GetType() != VariableType::kPrimitive) {
    // we don't cast complex structs and functions, also arrays, but can if we really want
    // however pointers - we do convert these, type punning go weeeeeeeeee
    return from->GetType() == VariableType::kPointer ? 0 : -1;
  }
  // which primitives we can cast to which
  if (!set_up_casting) SetUpCastingPrimitives();
  PrimitiveVariableType from_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(from)->GetPrimitiveType();
  PrimitiveVariableType to_pr = std::static_pointer_cast<TIDPrimitiveVariableType>(to)->GetPrimitiveType();
  return can_cast[static_cast<uint8_t>(from_pr)][static_cast<uint8_t>(to_pr)];
}

bool CanCastLossless(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValue(from, to) == 0;
}

bool CanCast(const std::shared_ptr<TIDVariableType> & from, const std::shared_ptr<TIDVariableType> & to) {
  return CastValue(from, to) != -1;
}

// Omits const and ref types, be careful with those
std::shared_ptr<TIDVariableType> LeastCommonType(const std::shared_ptr<TIDVariableType> & lhs,
    const std::shared_ptr<TIDVariableType> & rhs) {
  if (lhs == rhs) return lhs;
  if (CanCastLossless(lhs, rhs)) return rhs;
  if (CanCastLossless(rhs, lhs)) return lhs;
  if (lhs->GetType() != VariableType::kPrimitive || rhs->GetType() != VariableType::kPrimitive)
    return {};
  PrimitiveVariableType type1 = std::static_pointer_cast<TIDPrimitiveVariableType>(lhs)->GetPrimitiveType();
  PrimitiveVariableType type2 = std::static_pointer_cast<TIDPrimitiveVariableType>(rhs)->GetPrimitiveType();
  uint8_t utype1 = static_cast<uint8_t>(type1);
  uint8_t utype2 = static_cast<uint8_t>(type2);

  int32_t closest = 255;
  PrimitiveVariableType vert = PrimitiveVariableType::kUnknown;
  for (PrimitiveVariableType type : types) {
    auto utype = static_cast<uint8_t>(type);
    auto mid_type = GetPrimitiveVariableType(type);
    if (CanCastLossless(lhs, mid_type) && CanCastLossless(mid_type, rhs)
        && std::max(layer[utype1] - layer[utype], layer[utype2] - layer[utype])) {
      closest = std::max(layer[utype1] - layer[utype], layer[utype2] - layer[utype]);
      vert = type;
    }
  }
  return GetPrimitiveVariableType(vert);
}
