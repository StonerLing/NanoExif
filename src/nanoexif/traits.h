// SPDX-License-Identifier: MIT
//
// Type-traits helpers used by various NanoExif components.

#pragma once

#include <array>
#include <vector>

namespace nne {

// Detects std::array<T,N> at compile time.
template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

// Extracts the element type and size from an std::array.
template <typename T>
struct std_array_element;

template <typename T, std::size_t N>
struct std_array_element<std::array<T, N>> {
  using type = T;
  static constexpr std::size_t size = N;
};

// Detects std::vector<T> at compile time.
template <typename T>
struct is_vector : std::false_type {};
template <typename T>
struct is_vector<std::vector<T>> : std::true_type {};
}  // namespace nne
