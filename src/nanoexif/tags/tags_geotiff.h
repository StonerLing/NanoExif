// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string_view>

#include "nanoexif/types.h"

namespace nne {

enum GeoTiffTag : uint16_t {
  GTModelTypeGeoKey = 1024,
  GTRasterTypeGeoKey = 1025,
  GTCitationGeoKey = 1026,
  GeographicTypeGeoKey = 2048,
  GeogGeodeticDatumGeoKey = 2050,
  GeogPrimeMeridianGeoKey = 2051,
  GeogLinearUnitsGeoKey = 2052,
  GeogLinearUnitSizeGeoKey = 2053,
  GeogAngularUnitsGeoKey = 2054,
  GeogAngularUnitSizeGeoKey = 2055,
  GeogEllipsoidGeoKey = 2056,
  GeogSemiMajorAxisGeoKey = 2057,
  GeogSemiMinorAxisGeoKey = 2058,
  GeogInvFlatteningGeoKey = 2059,
  GeogAzimuthUnitsGeoKey = 2060,
  GeogPrimeMeridianLongGeoKey = 2061,
  ProjectedCSTypeGeoKey = 3072,
  ProjectionGeoKey = 3074,
  ProjCoordTransGeoKey = 3075,
  ProjLinearUnitsGeoKey = 3076,
  ProjLinearUnitSizeGeoKey = 3077,
  ProjStdParallel1GeoKey = 3078,
  ProjStdParallel2GeoKey = 3079,
  ProjNatOriginLongGeoKey = 3080,
  ProjNatOriginLatGeoKey = 3081,
  ProjFalseEastingGeoKey = 3082,
  ProjFalseNorthingGeoKey = 3083,
  ProjFalseOriginLongGeoKey = 3084,
  ProjFalseOriginLatGeoKey = 3085,
  ProjFalseOriginEastingGeoKey = 3086,
  ProjFalseOriginNorthingGeoKey = 3087,
  ProjCenterLongGeoKey = 3088,
  ProjCenterLatGeoKey = 3089,
  ProjCenterEastingGeoKey = 3090,
  ProjCenterNorthingGeoKey = 3091,
  ProjScaleAtNatOriginGeoKey = 3092,
  ProjScaleAtCenterGeoKey = 3093,
  ProjAzimuthAngleGeoKey = 3094,
  ProjStraightVertPoleLongGeoKey = 3095,
  VerticalCSTypeGeoKey = 4096,
  VerticalDatumGeoKey = 4097,
  VerticalUnitsGeoKey = 4099
};

enum GeoTiffTiffTag : uint16_t {
  GeoKeyDirectoryTag = 34735,
  GeoDoubleParamsTag = 34736,
  GeoAsciiParamsTag = 34737,
  ModelTiepointTag = 33922,
  ModelPixelScaleTag = 33550,
  ModelTransformationTag = 34264
};

enum GeoKeyHeaderOffset : uint16_t {  // NOLINT
  KeyDirectoryVersion = 0,
  KeyRevision = 1,
  MinorRevision = 2,
  NumberOfKeys = 3
};

template <GeoTiffTag tag>
struct GeoTiffTagTrait {};

template <>
struct GeoTiffTagTrait<GTModelTypeGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GTModelTypeGeoKey";
};
template <>
struct GeoTiffTagTrait<GTRasterTypeGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GTRasterTypeGeoKey";
};
template <>
struct GeoTiffTagTrait<GTCitationGeoKey> {
  using type = std::string;
  static constexpr std::string_view name = "GeoTIFF.GTCitationGeoKey";
};
template <>
struct GeoTiffTagTrait<GeographicTypeGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GeographicTypeGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogGeodeticDatumGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GeogGeodeticDatumGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogPrimeMeridianGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GeogPrimeMeridianGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogLinearUnitsGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GeogLinearUnitsGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogLinearUnitSizeGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.GeogLinearUnitSizeGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogAngularUnitsGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GeogAngularUnitsGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogAngularUnitSizeGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.GeogAngularUnitSizeGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogEllipsoidGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.GeogEllipsoidGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogSemiMajorAxisGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.GeogSemiMajorAxisGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogSemiMinorAxisGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.GeogSemiMinorAxisGeoKey";
};
template <>
struct GeoTiffTagTrait<GeogInvFlatteningGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.GeogInvFlatteningGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjectedCSTypeGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.ProjectedCSTypeGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjectionGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.ProjectionGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjCoordTransGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.ProjCoordTransGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjLinearUnitsGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.ProjLinearUnitsGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjLinearUnitSizeGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjLinearUnitSizeGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjStdParallel1GeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjStdParallel1GeoKey";
};
template <>
struct GeoTiffTagTrait<ProjStdParallel2GeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjStdParallel2GeoKey";
};
template <>
struct GeoTiffTagTrait<ProjNatOriginLongGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjNatOriginLongGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjNatOriginLatGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjNatOriginLatGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjFalseEastingGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjFalseEastingGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjFalseNorthingGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjFalseNorthingGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjFalseOriginLongGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjFalseOriginLongGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjFalseOriginLatGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjFalseOriginLatGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjFalseOriginEastingGeoKey> {
  using type = double;
  static constexpr std::string_view name =
      "GeoTIFF.ProjFalseOriginEastingGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjFalseOriginNorthingGeoKey> {
  using type = double;
  static constexpr std::string_view name =
      "GeoTIFF.ProjFalseOriginNorthingGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjCenterLongGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjCenterLongGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjCenterLatGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjCenterLatGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjCenterEastingGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjCenterEastingGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjCenterNorthingGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjCenterNorthingGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjScaleAtNatOriginGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjScaleAtNatOriginGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjScaleAtCenterGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjScaleAtCenterGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjAzimuthAngleGeoKey> {
  using type = double;
  static constexpr std::string_view name = "GeoTIFF.ProjAzimuthAngleGeoKey";
};
template <>
struct GeoTiffTagTrait<ProjStraightVertPoleLongGeoKey> {
  using type = double;
  static constexpr std::string_view name =
      "GeoTIFF.ProjStraightVertPoleLongGeoKey";
};
template <>
struct GeoTiffTagTrait<VerticalCSTypeGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.VerticalCSTypeGeoKey";
};
template <>
struct GeoTiffTagTrait<VerticalDatumGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.VerticalDatumGeoKey";
};
template <>
struct GeoTiffTagTrait<VerticalUnitsGeoKey> {
  using type = uint16_t;
  static constexpr std::string_view name = "GeoTIFF.VerticalUnitsGeoKey";
};

}  // namespace nne
