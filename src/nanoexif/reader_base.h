// SPDX-License-Identifier: MIT
//
// ReaderBase: shared base class for all metadata readers.  Provides
// low-level byte reading helpers (Next, NextVector, FromBytes) and endian
// conversion utilities used by the format-specific specializations.

#pragma once

#include <cstddef>
#include <cstring>
#include <fstream>
#include <vector>

#include "nanoexif/endian.h"
#include "nanoexif/errors.h"
#include "nanoexif/metadata.h"
#include "nanoexif/traits.h"
#include "nanoexif/types.h"

namespace nne {

namespace detail {

// Throws std::out_of_range if offset+n exceeds buf.size().
inline void EnsureRange(const std::vector<std::byte>& buf,
                        std::size_t offset,
                        std::size_t n) {
  if (offset > buf.size() || n > buf.size() - offset) {
    throw std::out_of_range("FromBytes: read out of range");
  }
}

// Assembles an unsigned integer from a sequence of bytes, respecting the
// specified endianness.  U must be unsigned and wide enough to hold n bytes.
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

// Bit-casts an unsigned integer to a floating-point value of the same size.
template <typename F, typename U>
F AssembleFloatFromUint(U value) {
  static_assert(std::is_floating_point_v<F>, "F must be floating point");
  static_assert(std::is_unsigned_v<U>, "U must be unsigned integer");
  F float_value;
  std::memcpy(&float_value, &value, sizeof(F));
  return float_value;
}

// Converts a char vector to std::string.
inline std::string AsString(const std::vector<char>& chars) {
  return std::string{chars.cbegin(), chars.cend()};
}

}  // namespace detail

template <MetaFormat meta_format>
class ReaderBase {
 public:
  explicit ReaderBase(std::string_view path)
      : input_(path.data(), std::ios::binary), is_little_endian_(true) {};

  virtual Result<Metadata<meta_format>> Read() = 0;

 protected:
  // Reads sizeof(T) raw bytes directly from the file stream and returns them
  // as type T.  T must be trivially copyable.  Handles std::array via data().
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

  // Reads len elements of type T from the file stream into a vector.
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

  // Extracts a single value of type T from an in-memory byte buffer,
  // handling endian conversion.  Supports integral and floating-point types.
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

  // Extracts count elements of type T from an in-memory byte buffer.
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

  // Converts value to big-endian or little-endian from native byte order.
  template <typename T>
  [[nodiscard]] T AsBE(T value) {
    return is_little_endian_ ? ReverseBytes(value) : value;
  }

  template <typename T>
  [[nodiscard]] T AsLE(T value) {
    return is_little_endian_ ? value : ReverseBytes(value);
  }

  std::vector<char> ReadEntireFile() {
    input_.seekg(0, std::ios::end);
    const std::streamsize size = input_.tellg();
    if (size <= 0) {
      return {};
    }
    input_.seekg(0, std::ios::beg);
    std::vector<char> buf(static_cast<std::size_t>(size));
    input_.read(buf.data(), size);
    return buf;
  }

  std::ifstream input_;
  bool is_little_endian_;  // NOLINT
};

template <ImageFormat image_format, MetaFormat meta_format>
struct is_metaformat_supported : std::false_type {};
}  // namespace nne
