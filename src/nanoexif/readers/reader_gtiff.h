// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <istream>
#include <string>
#include <string_view>
#include <vector>

#include "nanoexif/endian.h"
#include "nanoexif/errors.h"
#include "nanoexif/metadata.h"
#include "nanoexif/reader.h"
#include "nanoexif/tags/tags_exif.h"
#include "nanoexif/tags/tags_gtiff.h"
#include "nanoexif/types.h"

namespace nne {

template <>
struct is_metaformat_supported<TIFF, GEOTIFF> : std::true_type {};

namespace detail {

inline std::string ExtractGeoAscii(
    const std::vector<uint16_t>& geo_key_dir,
    std::size_t key_entry_start,
    const std::string& geo_ascii_params) {
  const uint16_t offset = geo_key_dir[key_entry_start + 3];
  const uint16_t count = geo_key_dir[key_entry_start + 2];
  if (offset >= geo_ascii_params.size() ||
      static_cast<std::size_t>(offset) + count > geo_ascii_params.size()) {
    return {};
  }
  std::string result = geo_ascii_params.substr(offset, count);
  while (!result.empty() && (result.back() == '|' || result.back() == '\0')) {
    result.pop_back();
  }
  return result;
}

}  // namespace detail

template <ImageFormat image_format>
class Reader<GEOTIFF, image_format> : ReaderBase<GEOTIFF> {
 public:
  using MetadataT = Metadata<GEOTIFF>;
  using ResultT = Result<MetadataT>;

  explicit Reader(std::string_view path) : ReaderBase<GEOTIFF>(path) {}

  [[nodiscard]] Result<Metadata<GEOTIFF>> Read() noexcept override {
    if (!input_) {
      return make_result<MetadataT>(ErrorCode::COULD_NOT_OPEN_FILE);
    }

    const std::vector<char> buf = ReadEntireFile();
    if (buf.empty()) {
      return make_result<MetadataT>(ErrorCode::COULD_NOT_OPEN_FILE);
    }

    if (buf.size() < 8) {
      return make_result<MetadataT>(ErrorCode::INVALID_EXIF_HEADER);
    }

    is_little_endian_ = (buf[0] == 'I' && buf[1] == 'I');

    const auto get16 = [&](std::size_t off) -> uint16_t {
      if (off + 2 > buf.size()) {
        return 0;
      }
      if (is_little_endian_) {
        return static_cast<uint8_t>(buf[off]) |
               (static_cast<uint8_t>(buf[off + 1]) << 8);
      }
      return (static_cast<uint8_t>(buf[off]) << 8) |
             static_cast<uint8_t>(buf[off + 1]);
    };

    const auto get32 = [&](std::size_t off) -> uint32_t {
      if (off + 4 > buf.size()) {
        return 0;
      }
      if (is_little_endian_) {
        return static_cast<uint8_t>(buf[off]) |
               (static_cast<uint8_t>(buf[off + 1]) << 8) |
               (static_cast<uint8_t>(buf[off + 2]) << 16) |
               (static_cast<uint8_t>(buf[off + 3]) << 24);
      }
      return (static_cast<uint8_t>(buf[off]) << 24) |
             (static_cast<uint8_t>(buf[off + 1]) << 16) |
             (static_cast<uint8_t>(buf[off + 2]) << 8) |
             static_cast<uint8_t>(buf[off + 3]);
    };

    const auto get64 = [&](std::size_t off) -> uint64_t {
      if (off + 8 > buf.size()) {
        return 0;
      }
      if (is_little_endian_) {
        return static_cast<uint64_t>(static_cast<uint8_t>(buf[off])) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 1]))
                << 8) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 2]))
                << 16) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 3]))
                << 24) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 4]))
                << 32) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 5]))
                << 40) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 6]))
                << 48) |
               (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 7]))
                << 56);
      }
      return (static_cast<uint64_t>(static_cast<uint8_t>(buf[off])) << 56) |
             (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 1]))
              << 48) |
             (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 2]))
              << 40) |
             (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 3]))
              << 32) |
             (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 4]))
              << 24) |
             (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 5]))
              << 16) |
             (static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 6]))
              << 8) |
             static_cast<uint64_t>(static_cast<uint8_t>(buf[off + 7]));
    };

    const auto get_double = [&](std::size_t off) -> double {
      const uint64_t raw = get64(off);
      double result;
      std::memcpy(&result, &raw, sizeof(double));
      return result;
    };

    if (get16(2) != TiffMarker::FIXED_START) {
      return make_result<MetadataT>(ErrorCode::INVALID_EXIF_HEADER);
    }

    std::vector<uint16_t> geo_key_dir_data;
    std::vector<double> geo_double_params;
    std::string geo_ascii_params;
    std::vector<double> model_pixel_scale;
    std::vector<double> model_tiepoint;
    std::vector<double> model_transformation;

    uint32_t ifd_offset = get32(4);
    while (ifd_offset != 0 && ifd_offset + 2 <= buf.size()) {
      const uint16_t num_entries = get16(ifd_offset);
      for (uint16_t i = 0; i < num_entries; ++i) {
        const std::size_t entry_off = ifd_offset + 2 + (12 * i);
        if (entry_off + 12 > buf.size()) {
          break;
        }

        const uint16_t tag = get16(entry_off);
        const uint16_t type = get16(entry_off + 2);
        const uint32_t count = get32(entry_off + 4);

        std::size_t value_offset = entry_off + 8;
        const std::size_t value_size =
            count * kExifTypeNumBytes[std::min<std::size_t>(type, 12)];
        if (value_size > 4) {
          value_offset = get32(entry_off + 8);
        }

        switch (tag) {
          case GeoTiffTiffTag::GeoKeyDirectoryTag:
            if (type == ExifDataType::SHORT) {
              geo_key_dir_data.resize(count);
              for (uint32_t j = 0; j < count; ++j) {
                geo_key_dir_data[j] = get16(value_offset + (2ULL * j));
              }
            }
            break;
          case GeoTiffTiffTag::GeoDoubleParamsTag:
            if (type == ExifDataType::DOUBLE) {
              geo_double_params.resize(count);
              for (uint32_t j = 0; j < count; ++j) {
                geo_double_params[j] = get_double(value_offset + (8ULL * j));
              }
            }
            break;
          case GeoTiffTiffTag::GeoAsciiParamsTag:
            if (type == ExifDataType::ASCII) {
              if (value_offset + count <= buf.size()) {
                geo_ascii_params =
                    std::string(buf.data() + value_offset, count);
              }
            }
            break;
          case GeoTiffTiffTag::ModelPixelScaleTag:
            if (type == ExifDataType::DOUBLE && count >= 3) {
              model_pixel_scale.resize(count);
              for (uint32_t j = 0; j < count; ++j) {
                model_pixel_scale[j] = get_double(value_offset + (8ULL * j));
              }
            }
            break;
          case GeoTiffTiffTag::ModelTiepointTag:
            if (type == ExifDataType::DOUBLE && count >= 6) {
              model_tiepoint.resize(count);
              for (uint32_t j = 0; j < count; ++j) {
                model_tiepoint[j] = get_double(value_offset + (8ULL * j));
              }
            }
            break;
          case GeoTiffTiffTag::ModelTransformationTag:
            if (type == ExifDataType::DOUBLE && count >= 16) {
              model_transformation.resize(count);
              for (uint32_t j = 0; j < count; ++j) {
                model_transformation[j] = get_double(value_offset + (8ULL * j));
              }
            }
            break;
          default:
            break;
        }
      }

      if (ifd_offset + 2 + num_entries * 12 + 4 > buf.size()) {
        break;
      }
      ifd_offset = get32(ifd_offset + 2 + (num_entries * 12));
    }

    if (!model_pixel_scale.empty()) {
      metadata_.Insert(GeoTiffTiffTag::ModelPixelScaleTag,
                       model_pixel_scale);
    }
    if (!model_tiepoint.empty()) {
      metadata_.Insert(GeoTiffTiffTag::ModelTiepointTag, model_tiepoint);
    }
    if (!model_transformation.empty()) {
      metadata_.Insert(GeoTiffTiffTag::ModelTransformationTag,
                       model_transformation);
    }

    if (geo_key_dir_data.size() >= 4) {
      const uint16_t num_keys =
          geo_key_dir_data[GeoKeyHeaderOffset::NumberOfKeys];
      for (uint16_t k = 0; k < num_keys; ++k) {
        const std::size_t key_entry_start = 4 + (k * 4);
        if (key_entry_start + 4 > geo_key_dir_data.size()) {
          break;
        }

        const uint16_t key_id = geo_key_dir_data[key_entry_start];
        const uint16_t tiff_tag_location =
            geo_key_dir_data[key_entry_start + 1];
        const uint16_t count = geo_key_dir_data[key_entry_start + 2];
        const uint16_t value_offset =
            geo_key_dir_data[key_entry_start + 3];

        if (tiff_tag_location == 0) {
          metadata_.Insert(key_id, value_offset);
        } else if (tiff_tag_location ==
                   GeoTiffTiffTag::GeoDoubleParamsTag) {
          if (value_offset < geo_double_params.size()) {
            if (count == 1) {
              metadata_.Insert(key_id, geo_double_params[value_offset]);
            } else {
              std::vector<double> vals(count);
              for (uint16_t j = 0; j < count; ++j) {
                if (value_offset + j < geo_double_params.size()) {
                  vals[j] = geo_double_params[value_offset + j];
                }
              }
              metadata_.Insert(key_id, std::move(vals));
            }
          }
        } else if (tiff_tag_location ==
                   GeoTiffTiffTag::GeoAsciiParamsTag) {
          std::string value =
              detail::ExtractGeoAscii(geo_key_dir_data, key_entry_start,
                                      geo_ascii_params);
          metadata_.Insert(key_id, value);
        }
      }
    }

    return make_result<MetadataT>(metadata_);
  }

 private:
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

  MetadataT metadata_;
};

inline Result<Metadata<GEOTIFF>> ReadGeoTIFF(
    const std::filesystem::path& path) {
  std::string ext = path.extension().string();
  for (auto& c : ext) {
    if (c >= 'A' && c <= 'Z') {
      c += 32;
    }
  }

  if (ext == ".tif" || ext == ".tiff") {
    Reader<GEOTIFF, TIFF> reader(path.string());
    return reader.Read();
  }

  return make_result<Metadata<GEOTIFF>>(ErrorCode::UNSUPPORTED_FORMAT);
}

}  // namespace nne
