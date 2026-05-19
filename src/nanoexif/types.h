// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "nanoexif/traits.h"

namespace nne {
using rational_t = std::uint64_t;
inline double RationalToDouble(uint64_t packed, bool is_little_endian = true) {
  uint32_t low = static_cast<uint32_t>(packed & 0xFFFFFFFFU);
  uint32_t high = static_cast<uint32_t>(packed >> 32);

  uint32_t num = is_little_endian ? low : high;
  uint32_t den = is_little_endian ? high : low;

  if (den == 0U) {
    if (num == 0U) {
      return std::numeric_limits<double>::quiet_NaN();  // 0/0
    }
    return std::numeric_limits<double>::infinity();  // n/0
  }

  return static_cast<double>(num) / static_cast<double>(den);
}

inline double SRationalToDouble(int64_t packed, bool is_little_endian) {
  int32_t low = static_cast<int32_t>(packed & 0xFFFFFFFFLL);
  int32_t high = static_cast<int32_t>(packed >> 32);

  int32_t num = is_little_endian ? low : high;
  int32_t den = is_little_endian ? high : low;

  if (den == 0) {
    if (num == 0) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return (num >= 0 ? 1.0 : -1.0) * std::numeric_limits<double>::infinity();
  }

  return static_cast<double>(num) / static_cast<double>(den);
}

enum ImageFormat : uint8_t { AUTO, JPEG, TIFF };

enum MetaFormat : uint8_t { EXIF, XMP, GEOTIFF };

template <MetaFormat F>
struct Metakey {};

template <>
struct Metakey<EXIF> {
  using type = uint16_t;
};

template <>
struct Metakey<XMP> {
  using type = std::string;
};

template <>
struct Metakey<GEOTIFF> {
  using type = uint16_t;
};

using Metavalue = std::variant<std::monostate,
                               uint8_t,
                               std::vector<uint8_t>,
                               uint16_t,
                               std::vector<uint16_t>,
                               uint32_t,
                               std::vector<uint32_t>,
                               uint64_t,
                               std::vector<uint64_t>,
                               int8_t,
                               std::vector<int8_t>,
                               int16_t,
                               std::vector<int16_t>,
                               int32_t,
                               std::vector<int32_t>,
                               int64_t,
                               std::vector<int64_t>,
                               double,
                               std::vector<double>,
                               std::string>;

inline void PrintMetavalue(const Metavalue& value) {
  std::visit(
      [](const auto& val) {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, std::monostate>) {
          std::cout << "(empty)";
        } else if constexpr (is_vector<T>::value) {
          if (val.empty()) {
            std::cout << "[]";
            return;
          }
          std::cout << "[";
          for (auto& elem : val) {
            using E = std::decay_t<decltype(elem)>;
            if constexpr (std::is_same_v<E, uint8_t> ||
                          std::is_same_v<E, int8_t>) {
              std::cout << static_cast<int>(elem) << " ";
            } else {
              std::cout << elem << " ";
            }
          }
          std::cout << "\b]";
        } else {
          if constexpr (std::is_same_v<T, uint8_t> ||
                        std::is_same_v<T, int8_t>) {
            std::cout << static_cast<int>(val);
          } else {
            std::cout << val;
          }
        }
      },
      value);
}
}  // namespace nne
