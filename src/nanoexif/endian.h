// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <string>

namespace nne {

template <typename T>
T ReverseBytes(const T& data) {
  T reversed = data;
  std::reverse(reinterpret_cast<char*>(&reversed),
               reinterpret_cast<char*>(&reversed) + sizeof(T));
  return reversed;
}

inline bool IsNativeLittleEndian() {
  unsigned int num = 1;
  char* byte = (char*)&num;
  return *byte == 1;
}

inline bool IsNativeBigEndian() { return !IsNativeLittleEndian(); }

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

template <typename T1, typename T2>
bool EqualIgnoreEndian(const T1& data1, const T2& data2) {
  return data1 == data2 || data1 == ReverseBytes(data2);
};

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
