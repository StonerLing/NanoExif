// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#include "nanoexif/types.h"

namespace nne {

enum JpegMarker : std::uint16_t {
  SOF0 = 0xFFC0,
  SOF1 = 0xFFC1,
  SOF2 = 0xFFC2,
  SOF3 = 0xFFC3,
  DHT = 0xFFC4,
  SOF5 = 0xFFC5,
  SOF6 = 0xFFC6,
  SOF7 = 0xFFC7,
  JPG = 0xFFC8,
  SOF9 = 0xFFC9,
  SOF10 = 0xFFCA,
  SOF11 = 0xFFCB,
  DAC = 0xFFCC,
  SOF13 = 0xFFCD,
  SOF14 = 0xFFCE,
  SOF15 = 0xFFCF,
  RST0 = 0xFFD0,
  RST1 = 0xFFD1,
  RST2 = 0xFFD2,
  RST3 = 0xFFD3,
  RST4 = 0xFFD4,
  RST5 = 0xFFD5,
  RST6 = 0xFFD6,
  RST7 = 0xFFD7,
  SOI = 0xFFD8,
  EOI = 0xFFD9,
  SOS = 0xFFDA,
  DQT = 0xFFDB,
  DNL = 0xFFDC,
  DRI = 0xFFDD,
  DHP = 0xFFDE,
  EXP = 0xFFDF,
  APP0 = 0xFFE0,
  APP1 = 0xFFE1,
  APP2 = 0xFFE2,
  APP3 = 0xFFE3,
  APP4 = 0xFFE4,
  APP5 = 0xFFE5,
  APP6 = 0xFFE6,
  APP7 = 0xFFE7,
  APP8 = 0xFFE8,
  APP9 = 0xFFE9,
  APP10 = 0xFFEA,
  APP11 = 0xFFEB,
  APP12 = 0xFFEC,
  APP13 = 0xFFED,
  APP14 = 0xFFEE,
  APP15 = 0xFFEF,
  JPG0 = 0xFFF0,
  JPG1 = 0xFFF1,
  JPG2 = 0xFFF2,
  JPG3 = 0xFFF3,
  JPG4 = 0xFFF4,
  JPG5 = 0xFFF5,
  JPG6 = 0xFFF6,
  JPG7 = 0xFFF7,
  JPG8 = 0xFFF8,
  JPG9 = 0xFFF9,
  JPG10 = 0xFFFA,
  JPG11 = 0xFFFB,
  JPG12 = 0xFFFC,
  JPG13 = 0xFFFD,
  COM = 0xFFFE
};

enum TiffMarker : std::uint16_t {
  LITTLE_ENDIAN = 0x4949,
  BIG_ENDIAN = 0x4D4D,
  FIXED_START = 0x002A
};

enum ExifDataType : std::uint16_t {  // NOLINT
  BYTE = 0x1,
  ASCII = 0x2,
  SHORT = 0x3,
  LONG = 0x4,
  RATIONAL = 0x5,
  SBYTE = 0x6,
  UNDEFINED = 0x7,
  SSHORT = 0x8,
  SLONG = 0x9,
  SRATIONAL = 0xA
};
static constexpr std::array<std::size_t, 11> kExifTypeNumBytes{
    0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8};

template <ExifDataType type>
struct ExifTypeTrait {};

template <>
struct ExifTypeTrait<BYTE> {
  using type = std::uint8_t;
  static constexpr std::size_t size = 1;
};
template <>
struct ExifTypeTrait<ASCII> {
  using type = char;
  static constexpr std::size_t size = 1;
};
template <>
struct ExifTypeTrait<SHORT> {
  using type = uint16_t;
  static constexpr std::size_t size = 2;
};
template <>
struct ExifTypeTrait<LONG> {
  using type = uint32_t;
  static constexpr std::size_t size = 4;
};
template <>
struct ExifTypeTrait<RATIONAL> {
  using type = uint64_t;
  static constexpr std::size_t size = 8;
};
template <>
struct ExifTypeTrait<SBYTE> {
  using type = int8_t;
  static constexpr std::size_t size = 1;
};
template <>
struct ExifTypeTrait<UNDEFINED> {
  using type = uint8_t;
  static constexpr std::size_t size = 1;
};
template <>
struct ExifTypeTrait<SSHORT> {
  using type = int16_t;
  static constexpr std::size_t size = 2;
};
template <>
struct ExifTypeTrait<SLONG> {
  using type = int32_t;
  static constexpr std::size_t size = 4;
};
template <>
struct ExifTypeTrait<SRATIONAL> {
  using type = int64_t;
  static constexpr std::size_t size = 8;
};

enum ExifTag : uint16_t {
  PixelXDimension = 0xA002,
  PixelYDimension = 0xA003,
  ImageDescription = 0x010E,
  Make = 0x010F,
  Model = 0x0110,
  Orientation = 0x0112,
  XResolution = 0x011A,
  YResolution = 0x011B,
  ResolutionUnit = 0x0128,
  ExifIfdOffset = 0x8769,
  GPSIfdOffset = 0x8825,
  CreateDate = 0x9004,
  FocalLength = 0x920A,
  FocalLengthIn35mmFilm = 0xA405,
  GPSLatitudeRef = 0x0001,
  GPSLatitude = 0x0002,
  GPSLongitudeRef = 0x0003,
  GPSLongitude = 0x0004,
  GPSAltitudeRef = 0x0005,
  GPSAltitude = 0x0006
};

template <ExifTag tag>
struct ExifTagTrait {
  static constexpr uint16_t value = tag;
};

template <>
struct ExifTagTrait<PixelXDimension> {
  using type = uint16_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.PixelXDimension";
};
template <>
struct ExifTagTrait<PixelYDimension> {
  using type = uint16_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.PixelYDimension";
};
template <>
struct ExifTagTrait<ImageDescription> {
  using type = std::string;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.ImageDescription";
};
template <>
struct ExifTagTrait<Make> {
  using type = std::string;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.Make";
};
template <>
struct ExifTagTrait<Model> {
  using type = std::string;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.Model";
};
template <>
struct ExifTagTrait<Orientation> {
  using type = std::uint16_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.Orientation";
};
template <>
struct ExifTagTrait<XResolution> {
  using type = double;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.XResolution";
};
template <>
struct ExifTagTrait<YResolution> {
  using type = double;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.YResolution";
};
template <>
struct ExifTagTrait<ResolutionUnit> {
  using type = std::uint16_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.ResolutionUnit";
};
template <>
struct ExifTagTrait<ExifIfdOffset> {
  using type = std::uint32_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.ExifIfdOffset";
};
template <>
struct ExifTagTrait<GPSIfdOffset> {
  using type = std::uint32_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.GPSIfdOffset";
};
template <>
struct ExifTagTrait<CreateDate> {
  using type = std::string;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.CreateDate";
};
template <>
struct ExifTagTrait<FocalLength> {
  using type = double;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.FocalLength";
};
template <>
struct ExifTagTrait<FocalLengthIn35mmFilm> {
  using type = uint16_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.FocalLengthIn35mmFilm";
};
template <>
struct ExifTagTrait<GPSLatitudeRef> {
  using type = std::string;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.GPSLatitudeRef";
};
template <>
struct ExifTagTrait<GPSLatitude> {
  using type = std::vector<double>;
  static constexpr std::size_t size = 3;
  static constexpr std::string_view name = "Exif.GPSLatitude";
};
template <>
struct ExifTagTrait<GPSLongitudeRef> {
  using type = std::string;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.GPSLongitudeRef";
};
template <>
struct ExifTagTrait<GPSLongitude> {
  using type = std::vector<double>;
  static constexpr std::size_t size = 3;
  static constexpr std::string_view name = "Exif.GPSLongitude";
};
template <>
struct ExifTagTrait<GPSAltitudeRef> {
  using type = int32_t;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.GPSAltitudeRef";
};
template <>
struct ExifTagTrait<GPSAltitude> {
  using type = double;
  static constexpr std::size_t size = 1;
  static constexpr std::string_view name = "Exif.GPSAltitude";
};
}  // namespace nne
