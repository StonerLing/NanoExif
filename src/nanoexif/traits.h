// SPDX-License-Identifier: MIT

#include <array>
#pragma once

namespace nne {
template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
struct std_array_element;

template <typename T, std::size_t N>
struct std_array_element<std::array<T, N>> {
  using type = T;
  static constexpr std::size_t size = N;
};
}  // namespace nne
