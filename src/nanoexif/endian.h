// SPDX-License-Identifier: MIT
//
// Endianness utilities: detect host byte order, swap bytes, and compare
// values regardless of byte order.

#pragma once

#include <algorithm>
#include <string>

namespace nne {

// Reverses the byte order of any trivially-copyable type T by treating its
// memory as a char array and calling std::reverse.
template <typename T>
T ReverseBytes(const T& data) {
  T reversed = data;
  std::reverse(reinterpret_cast<char*>(&reversed),
               reinterpret_cast<char*>(&reversed) + sizeof(T));
  return reversed;
}

// Detects host endianness at runtime by inspecting the first byte of an
// integer.  (C++20 provides std::endian; for C++17 a runtime check suffices.)
inline bool IsNativeLittleEndian() {
  unsigned int num = 1;
  char* byte = (char*)&num;
  return *byte == 1;
}

inline bool IsNativeBigEndian() { return !IsNativeLittleEndian(); }

// Converts between little/big-endian and native byte order.
template <typename T>
T LittleEndianToNative(T data) {
  return IsNativeLittleEndian() ? data : ReverseBytes(data);
}

template <typename T>
T BigEndianToNative(T data) {
  return IsNativeBigEndian() ? data : ReverseBytes(data);
}

template <typename T>
T NativeToLittleEndian(T data) {
  return IsNativeLittleEndian() ? data : ReverseBytes(data);
}

template <typename T>
T NativeToBigEndian(T data) {
  return IsNativeBigEndian() ? data : ReverseBytes(data);
}

// Returns true if data1 equals data2 in either byte order.
template <typename T1, typename T2>
bool EqualIgnoreEndian(const T1& data1, const T2& data2) {
  return data1 == data2 || data1 == ReverseBytes(data2);
};

// Overload for comparing a runtime buffer against a compile-time string
// literal, checking both native and reversed byte orders.
template <std::size_t N>
bool EqualIgnoreEndian(const char* data1,
                       std::size_t len,
                       const char (&data2)[N]) {  // NOLINT
  if (len != N) {
    return false;
  }

  if (std::memcmp(data1, data2, N) == 0) {
    return true;
  }

  std::array<char, N> reversed;
  for (std::size_t i = 0; i < N; ++i) {
    reversed[i] = data2[N - 1 - i];
  }
  return std::memcmp(data1, reversed.data(), N) == 0;
}

}  // namespace nne
