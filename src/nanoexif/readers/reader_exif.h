// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <istream>
#include <utility>
#include <vector>
#include <filesystem>

#include "nanoexif/reader.h"
#include "nanoexif/tags/tags_exif.h"
#include "nanoexif/types.h"

namespace nne {

template <>
struct is_metaformat_supported<JPEG, EXIF> : std::true_type {};
template <>
struct is_metaformat_supported<TIFF, EXIF> : std::true_type {};
template <>
struct is_metaformat_supported<GEOTIFF, EXIF> : std::true_type {};

template <ImageFormat image_format>
class Reader<EXIF, image_format> : ReaderBase<EXIF> {
 public:
  using MetadataT = Metadata<EXIF>;
  using ResultT = Result<MetadataT>;

  explicit Reader(std::string_view path) : ReaderBase<EXIF>(path) {};

  [[nodiscard]] Result<Metadata<EXIF>> Read() noexcept override {
    if (!input_) {
      return make_result<MetadataT>(ErrorCode::COULD_NOT_OPEN_FILE);
    }

    auto is_format_valid = ValidateFormat();
    if (!is_format_valid) {
      return make_result<MetadataT>(is_format_valid.error());
    }

    ReadTiffHeader();

    is_little_endian_ = detail::AsString(Seek<char>(0, 2)) == "II";

    if (Seek<uint16_t>(2) != TiffMarker::FIXED_START) {
      return make_result<MetadataT>(ErrorCode::INVALID_EXIF_HEADER);
    }

    uint32_t ifd_offset = Seek<uint32_t>(4);
    while (ifd_offset != 0) {
      ifd_offset = ParseIfd(ifd_offset);
    }

    if (metadata_.Has(ExifTag::ExifIfdOffset)) {
      ParseIfd(metadata_.Get<ExifIfdOffset>());
    }

    if (metadata_.Has(ExifTag::GPSIfdOffset)) {
      ParseIfd(metadata_.Get<GPSIfdOffset>());
    }

    return make_result<MetadataT>(metadata_);
  };

 private:
  Result<bool> ValidateFormat() {
    if constexpr (image_format == JPEG) {
      if (!EqualIgnoreEndian(Next<uint16_t>(), JpegMarker::SOI)) {
        return make_result<bool>(ErrorCode::INVALID_JPEG_HEADER);
      }

      if (!EqualIgnoreEndian(Next<uint16_t>(), JpegMarker::APP1)) {
        return make_result<bool>(ErrorCode::INVALID_EXIF_HEADER);
      }

      total_bytes_ = Next<uint16_t>() - 8;  // include itself

      if (!EqualIgnoreEndian(Next<std::array<char, 6>>().data(), 6, "Exif\0")) {
        return make_result<bool>(ErrorCode::INVALID_EXIF_HEADER);
      }
    } else if constexpr (image_format == TIFF || image_format == GEOTIFF) {
      return make_result(true);
    } else {
      return make_result<false>(ErrorCode::UNSUPPORTED_FORMAT);
    }

    return make_result(true);
  }

  void ReadTiffHeader() {
    if constexpr (image_format == JPEG) {
      tiff_header_ = NextVector<std::byte>(total_bytes_);
    } else {
      input_.seekg(0, std::ios::end);
      std::streamsize total_bytes_ = input_.tellg();
      input_.seekg(0, std::ios::beg);
      tiff_header_ = NextVector<std::byte>(total_bytes_);
    }

    input_.clear();
    input_.seekg(0, std::ios::beg);
  }

  uint32_t ParseIfd(uint32_t ifd_offset) {
    uint16_t num_entries = Seek<uint16_t>(ifd_offset);

    for (int i = 0; i < num_entries; ++i) {
      std::size_t entry_offset = ifd_offset + 2 + (12 * i);

      uint16_t tag = Seek<uint16_t>(entry_offset);
      entry_offset += 2;

      uint16_t value_type = Seek<uint16_t>(entry_offset);
      entry_offset += 2;

      uint32_t value_count = Seek<uint32_t>(entry_offset);
      entry_offset += 4;

      uint32_t value_offset = entry_offset;
      bool is_field_offset = value_count * kExifTypeNumBytes[value_type] > 4;
      if (is_field_offset) {
        value_offset = Seek<uint32_t>(entry_offset);
      }

      Metavalue value;
      switch (value_type) {
        case ExifDataType::ASCII:
          value = detail::AsString(Seek<char>(value_offset, value_count));
          break;
        case ExifDataType::BYTE:
          if (value_count == 1) {
            value = Seek<uint8_t>(value_offset);
          } else {
            value = Seek<uint8_t>(value_offset, value_count);
          }
          break;
        case ExifDataType::SHORT:
          if (value_count == 1) {
            value = Seek<uint16_t>(value_offset);
          } else {
            value = Seek<uint16_t>(value_offset, value_count);
          }
          break;
        case ExifDataType::LONG:
          if (value_count == 1) {
            value = Seek<uint32_t>(value_offset);
          } else {
            value = Seek<uint32_t>(value_offset, value_count);
          }
          break;
        case ExifDataType::RATIONAL:
          if (value_count == 1) {
            value = RationalToDouble(Seek<rational_t>(value_offset),
                                     is_little_endian_);
          } else {
            auto raw = Seek<rational_t>(value_offset, value_count);
            std::vector<double> conv;
            conv.reserve(raw.size());
            for (auto& r : raw) {  // NOLINT
              conv.push_back(RationalToDouble(r, is_little_endian_));
            }
            value = std::move(conv);
          }
          break;
        case ExifDataType::SBYTE:
          if (value_count == 1) {
            value = Seek<int8_t>(value_offset);
          } else {
            value = Seek<int8_t>(value_offset, value_count);
          }
          break;
        case ExifDataType::SSHORT:
          if (value_count == 1) {
            value = Seek<int16_t>(value_offset);
          } else {
            value = Seek<int16_t>(value_offset, value_count);
          }
          break;
        case ExifDataType::SLONG:
          if (value_count == 1) {
            value = Seek<int32_t>(value_offset);
          } else {
            value = Seek<int32_t>(value_offset, value_count);
          }
          break;
        case ExifDataType::SRATIONAL:
          if (value_count == 1) {
            value = SRationalToDouble(Seek<int64_t>(value_offset),
                                      is_little_endian_);
          } else {
            auto raw = Seek<int64_t>(value_offset, value_count);
            std::vector<double> conv;
            conv.reserve(raw.size());
            for (auto& r : raw) {  // NOLINT
              conv.push_back(SRationalToDouble(r, is_little_endian_));
            }
            value = std::move(conv);
          }
          break;
        case ExifDataType::UNDEFINED:
          if (value_count == 1) {
            value = Seek<uint8_t>(value_offset);
          } else {
            value = Seek<uint8_t>(value_offset, value_count);
          }
          break;
        default:
          value = std::string("Unknown data type");
          break;
      }
      metadata_.Insert(tag, value);
    }

    return 0;
  }

  template <typename T>
  auto Seek(std::size_t offset) {
    return FromBytes<T>(tiff_header_, offset);
  }

  template <typename T>
  auto Seek(std::size_t offset, std::size_t count) {
    return FromBytes<T>(tiff_header_, offset, count);
  }

  MetadataT metadata_;
  uint32_t total_bytes_;
  std::vector<std::byte> tiff_header_;
};

inline Result<Metadata<EXIF>> ReadEXIF(const std::filesystem::path& path) {
  std::string ext = path.extension().string();
  for (auto& c : ext) {  // NOLINT
    if (c >= 'A' && c <= 'Z') {
      c += 32;
    }
  }

  if (ext == ".jpg" || ext == ".jpeg") {
    Reader<EXIF, JPEG> reader(path.string());
    return reader.Read();
  }
  if (ext == ".tif" || ext == ".tiff") {
    Reader<EXIF, TIFF> reader(path.string());
    return reader.Read();
  }

  return make_result<Metadata<EXIF>>(ErrorCode::UNSUPPORTED_FORMAT);
}
}  // namespace nne
