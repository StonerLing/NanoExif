// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <fstream>
#include <vector>

#include "nanoexif/endian.h"
#include "nanoexif/errors.h"
#include "nanoexif/metadata.h"
#include "nanoexif/traits.h"
#include "nanoexif/types.h"


namespace nne {

namespace detail {

inline void EnsureRange(const std::vector<std::byte>& buf,
                        std::size_t offset,
                        std::size_t n) {
  if (offset > buf.size() || n > buf.size() - offset) {
    throw std::out_of_range("FromBytes: read out of range");
  }
}

template <typename U>
U AssembleUnsignedFromBytes(const std::vector<std::byte>& buf,
                            std::size_t offset,
                            std::size_t n,
                            bool is_little_endian) {
  static_assert(std::is_unsigned_v<U>, "U must be unsigned");
  if (n == 0) {
    return U{0};
  }
  EnsureRange(buf, offset, n);

  U value = 0;
  if (is_little_endian) {
    for (std::size_t i = 0; i < n; ++i) {
      value |= (static_cast<U>(static_cast<unsigned char>(buf[offset + i]))
                << (8 * i));
    }
  } else {
    for (std::size_t i = 0; i < n; ++i) {
      value = (value << 8) |
              static_cast<U>(static_cast<unsigned char>(buf[offset + i]));
    }
  }
  return value;
}

template <typename F, typename U>
F AssembleFloatFromUint(U value) {
  static_assert(std::is_floating_point_v<F>, "F must be floating point");
  static_assert(std::is_unsigned_v<U>, "U must be unsigned integer");
  F float_value;
  std::memcpy(&float_value, &value, sizeof(F));
  return float_value;
}

inline std::string AsString(const std::vector<char>& chars) {
  return std::string{chars.cbegin(), chars.cend()};
}

}  // namespace detail

template <MetaFormat meta_format>
class ReaderBase {
 public:
  explicit ReaderBase(std::string_view path)
      : input_(path.data(), std::ios::binary) {};

  virtual Result<Metadata<meta_format>> Read() = 0;

 protected:
  template <typename T>
  [[nodiscard]] T Next() {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable to read raw bytes");

    T value;

    if constexpr (is_std_array<T>::value) {
      input_.read(reinterpret_cast<char*>(std::data(value)), sizeof(T));
      return value;
    } else {
      input_.read(reinterpret_cast<char*>(&value), sizeof(T));
      return value;
    }
  }

  template <typename T>
  [[nodiscard]] std::vector<T> NextVector(std::size_t len) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable to read raw bytes");

    std::vector<T> buffer;
    buffer.resize(len);
    if (len > 0) {
      input_.read(reinterpret_cast<char*>(buffer.data()),
                  static_cast<std::streamsize>(len * sizeof(T)));
    }
    return buffer;
  }

  template <typename T>
  T FromBytes(const std::vector<std::byte>& buffer, std::size_t offset) {
    if constexpr (std::is_integral_v<T>) {
      constexpr std::size_t nbytes = sizeof(T);
      detail::EnsureRange(buffer, offset, nbytes);

      auto assembled = detail::AssembleUnsignedFromBytes<std::uint64_t>(
          buffer, offset, nbytes, is_little_endian_);

      return static_cast<T>(assembled);
    } else if constexpr (std::is_floating_point_v<T>) {
      constexpr std::size_t nbytes = sizeof(T);
      detail::EnsureRange(buffer, offset, nbytes);

      using uint_t =
          std::conditional_t<sizeof(T) == 4, std::uint32_t, std::uint64_t>;
      uint_t assembled = detail::AssembleUnsignedFromBytes<uint_t>(
          buffer, offset, nbytes, is_little_endian_);
      return detail::AssembleFloatFromUint<T, uint_t>(assembled);
    } else {
      static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
                    "FromBytes<T> only supports integral and floating point "
                    "types by this overload. "
                    "For raw bytes use FromBytes<unsigned char>(..., count) to "
                    "get bytes.");
    }
  }

  template <typename T>
  std::vector<T> FromBytes(const std::vector<std::byte>& buf,
                           std::size_t offset,
                           std::size_t count) {
    if (count == 0) {
      return {};
    }

    constexpr std::size_t elem_size = sizeof(T);
    const std::size_t total_bytes = elem_size * count;
    detail::EnsureRange(buf, offset, total_bytes);

    std::vector<T> out;
    out.resize(count);

    for (std::size_t i = 0; i < count; ++i) {
      std::size_t elem_offset = offset + (i * elem_size);
      out[i] = FromBytes<T>(buf, elem_offset);
    }

    return out;
  }

  template <typename T>
  [[nodiscard]] T AsBE(T value) {
    return is_little_endian_ ? ReverseBytes(value) : value;
  }

  template <typename T>
  [[nodiscard]] T AsLE(T value) {
    return is_little_endian_ ? value : ReverseBytes(value);
  }

  std::ifstream input_;
  bool is_little_endian_;
};

template <ImageFormat image_format, MetaFormat meta_format>
struct is_metaformat_supported : std::false_type {};
}  // namespace nne
