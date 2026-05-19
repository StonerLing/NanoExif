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

#include "nanoexif/detail/xml.h"
#include "nanoexif/endian.h"
#include "nanoexif/errors.h"
#include "nanoexif/metadata.h"
#include "nanoexif/reader.h"
#include "nanoexif/tags/tags_exif.h"
#include "nanoexif/tags/tags_xmp.h"
#include "nanoexif/types.h"

namespace nne {

template <>
struct is_metaformat_supported<JPEG, XMP> : std::true_type {};
template <>
struct is_metaformat_supported<TIFF, XMP> : std::true_type {};

namespace detail {

inline std::string FindXmpPacket(const std::vector<char>& buf) {
  static constexpr std::string_view xmp_pkt_start = "<?xpacket begin=";
  static constexpr std::string_view xmp_pkt_end = "<?xpacket end=";

  const auto* pos = buf.data();

  const auto* start = std::search(
      pos, pos + buf.size(), xmp_pkt_start.begin(), xmp_pkt_start.end());
  if (start == pos + buf.size()) {
    return {};
  }

  const auto* end = std::search(
      start + 1, pos + buf.size(), xmp_pkt_end.begin(), xmp_pkt_end.end());
  if (end == pos + buf.size()) {
    return {start, pos + buf.size() - (start - pos)};
  }

  const auto* close =
      std::find(end + xmp_pkt_end.size(), pos + buf.size(), '>');
  if (close != pos + buf.size()) {
    return {start, static_cast<std::size_t>(close + 1 - start)};
  }

  return {start, pos + buf.size() - (start - pos)};
}

inline void ExtractXmpAttributes(const detail::XMLElement& elem,
                                 Metadata<XMP>& metadata) {
  for (const auto& attr : elem.attributes) {
    if (attr.name.find("xmlns") != 0 && attr.name.find("rdf:") != 0) {
      metadata.Insert(attr.name, attr.value);
    }
  }

  for (const auto& child : elem.children) {
    if (child.name.find("rdf:") == 0) {
      ExtractXmpAttributes(child, metadata);
      continue;
    }
    if (!child.children.empty() || !child.text.empty()) {
      std::string text = child.text;
      for (const auto& grandchild : child.children) {
        if (!grandchild.text.empty()) {
          if (!text.empty()) {
            text += ", ";
          }
          text += grandchild.text;
        }
        for (const auto& gc2 : grandchild.children) {
          if (!gc2.text.empty()) {
            if (!text.empty()) {
              text += ", ";
            }
            text += gc2.text;
          }
        }
      }
      if (!text.empty()) {
        metadata.Insert(child.name, text);
      }
    }
    ExtractXmpAttributes(child, metadata);
  }
}

}  // namespace detail

template <ImageFormat image_format>
class Reader<XMP, image_format> : ReaderBase<XMP> {
 public:
  using MetadataT = Metadata<XMP>;
  using ResultT = Result<MetadataT>;

  explicit Reader(std::string_view path) : ReaderBase<XMP>(path) {}

  [[nodiscard]] Result<Metadata<XMP>> Read() noexcept override {
    if (!input_) {
      return make_result<MetadataT>(ErrorCode::COULD_NOT_OPEN_FILE);
    }

    const std::vector<char> buf = ReadEntireFile();
    if (buf.empty()) {
      return make_result<MetadataT>(ErrorCode::COULD_NOT_OPEN_FILE);
    }

    std::string xmp_xml;
    if constexpr (image_format == JPEG) {
      xmp_xml = ExtractXmpFromJpeg(buf);
    } else {
      xmp_xml = ExtractXmpFromTiff(buf);
    }

    if (xmp_xml.empty()) {
      return make_result<MetadataT>(ErrorCode::XMP_NOT_FOUND);
    }

    auto parse_result = detail::ParseXML(xmp_xml);
    if (!parse_result) {
      return make_result<MetadataT>(parse_result.error());
    }
    const detail::XMLElement& root = *parse_result;

    const auto* xmpmeta = root.FindChild("x:xmpmeta");
    if (xmpmeta == nullptr) {
      xmpmeta = root.FindChild("xmpmeta");
    }
    if (xmpmeta == nullptr) {
      for (const auto& child : root.children) {
        if (child.name.find("xmpmeta") != std::string::npos) {
          xmpmeta = &child;
          break;
        }
      }
    }

    if (xmpmeta != nullptr) {
      const auto* rdf = xmpmeta->FindChildByQualified("rdf", "RDF");
      if (rdf != nullptr) {
        for (const auto& desc : rdf->children) {
          if (desc.name == "rdf:Description" ||
              desc.name.find("Description") != std::string::npos) {
            detail::ExtractXmpAttributes(desc, metadata_);
          }
        }
      }
      detail::ExtractXmpAttributes(*xmpmeta, metadata_);
    } else {
      for (const auto& child : root.children) {
        if (child.name == "rdf:RDF") {
          for (const auto& desc : child.children) {
            if (desc.name == "rdf:Description") {
              detail::ExtractXmpAttributes(desc, metadata_);
            }
          }
        }
      }
      detail::ExtractXmpAttributes(root, metadata_);
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

  [[nodiscard]] std::string ExtractXmpFromJpeg(
      const std::vector<char>& buf) const {
    static constexpr std::string_view xmp_id = "http://ns.adobe.com/xap/1.0/";
    std::size_t i = 0;

    while (i + 1 < buf.size()) {
      if (static_cast<unsigned char>(buf[i]) != 0xFF) {
        ++i;
        continue;
      }

      const uint8_t marker = static_cast<uint8_t>(buf[i + 1]);
      if (marker == 0xD8) {
        i += 2;
        continue;
      }
      if (marker == 0xD9) {
        break;
      }

      if (i + 3 >= buf.size()) {
        break;
      }
      const uint16_t seg_size = (static_cast<uint8_t>(buf[i + 2]) << 8) |
                                static_cast<uint8_t>(buf[i + 3]);

      if (marker == 0xE1 && seg_size >= xmp_id.size() + 2) {
        const std::string_view id(
            &buf[i + 4],
            std::min<std::size_t>(seg_size - 2, buf.size() - i - 4));
        if (id.substr(0, xmp_id.size()) == xmp_id) {
          std::size_t data_start = xmp_id.size();
          if (data_start < id.size() && id[data_start] == '\0') {
            ++data_start;
          }
          return std::string(id.substr(data_start, id.size() - data_start));
        }
      }

      i += 2 + seg_size;
    }

    return detail::FindXmpPacket(buf);
  }

  std::string ExtractXmpFromTiff(const std::vector<char>& buf) {
    if (buf.size() < 8) {
      return {};
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

    static constexpr uint16_t kTagXmp = 700;
    uint32_t ifd_offset = get32(4);

    while (ifd_offset != 0 && ifd_offset + 2 <= buf.size()) {
      const uint16_t num_entries = get16(ifd_offset);
      for (uint16_t i = 0; i < num_entries; ++i) {
        const std::size_t entry_off = ifd_offset + 2 + (12 * i);
        if (entry_off + 12 > buf.size()) {
          break;
        }

        const uint16_t tag = get16(entry_off);
        if (tag != kTagXmp) {
          continue;
        }

        const uint16_t type = get16(entry_off + 2);
        const uint32_t count = get32(entry_off + 4);

        if (type == 2) {
          const std::size_t total_bytes = static_cast<std::size_t>(count);
          uint32_t data_off = get32(entry_off + 8);
          if (total_bytes <= 4) {
            data_off = static_cast<uint32_t>(entry_off + 8);
            if (data_off + total_bytes > buf.size()) {
              break;
            }
            return {buf.data() + data_off, buf.data() + data_off + total_bytes};
          }
          if (data_off + total_bytes > buf.size()) {
            break;
          }
          return {buf.data() + data_off, buf.data() + data_off + total_bytes};
        }
        break;
      }

      if (ifd_offset + 2 + num_entries * 12 + 4 > buf.size()) {
        break;
      }
      ifd_offset = get32(ifd_offset + 2 + (num_entries * 12));
    }

    return detail::FindXmpPacket(buf);
  }

  MetadataT metadata_;
};

inline Result<Metadata<XMP>> ReadXMP(const std::filesystem::path& path) {
  std::string ext = path.extension().string();
  for (auto& c : ext) {
    if (c >= 'A' && c <= 'Z') {
      c += 32;
    }
  }

  if (ext == ".jpg" || ext == ".jpeg") {
    Reader<XMP, JPEG> reader(path.string());
    return reader.Read();
  }
  if (ext == ".tif" || ext == ".tiff") {
    Reader<XMP, TIFF> reader(path.string());
    return reader.Read();
  }
  if (ext == ".xmp") {
    Reader<XMP, JPEG> reader(path.string());
    return reader.Read();
  }

  return make_result<Metadata<XMP>>(ErrorCode::UNSUPPORTED_FORMAT);
}

}  // namespace nne
