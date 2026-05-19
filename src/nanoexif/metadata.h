// SPDX-License-Identifier: MIT
//
// Generic metadata container templated by metadata format.

#pragma once

#include "nanoexif/tags/tags_exif.h"
#include "nanoexif/types.h"

namespace nne {

// Stores parsed metadata as a key-value map.  Key type depends on MetaFormat
// (uint16_t for EXIF/GeoTIFF, std::string for XMP).  Values are always
// Metavalue variants.  The Get<ExifTag>() overload provides typed access for
// known EXIF tags via ExifTagTrait.
template <MetaFormat meta_format>
class Metadata {
  using key_type = typename Metakey<meta_format>::type;
  using value_type = Metavalue;

  using map_type = std::unordered_map<key_type, value_type>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

 public:
  void Insert(key_type key, value_type value) { data_[key] = value; }
  void InsertOrAssign(key_type key, value_type value) {
    data_.insert_or_assign(key, value);
  }
  bool Has(key_type key) const { return data_.count(key) > 0; }

  Metavalue Get(key_type key) const {
    if (!Has(key)) {
      return Metavalue{};
    }
    return data_.at(key);
  }

  // Typed accessor for known EXIF tags.  Returns a default-constructed value
  // (numeric zero or empty string) if the tag is not present.
  template <ExifTag tag>
  auto Get() const {
    using value_type = typename ExifTagTrait<tag>::type;

    if (!Has(tag)) {
      return value_type(0);
    }
    return std::get<value_type>(data_.at(tag));
  }

  std::unordered_map<key_type, value_type>& data() { return data_; }
  const std::unordered_map<key_type, value_type>& data() const { return data_; }

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }

 private:
  std::unordered_map<key_type, value_type> data_;
};

}  // namespace nne
