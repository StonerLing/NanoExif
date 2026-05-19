// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

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

enum ImageFormat : uint8_t { AUTO, JPEG, TIFF, GEOTIFF };

enum MetaFormat : uint8_t { EXIF, XMP };

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
                               std::string>;
}  // namespace nne
