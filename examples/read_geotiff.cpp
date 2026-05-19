// SPDX-License-Identifier: MIT
//
// Example: read and display GeoTIFF metadata from a TIFF image.

#include <iomanip>
#include <iostream>
#include <filesystem>

#include "nanoexif/readers/reader_gtiff.h"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace nne;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: read_geotiff <tiff_image_path>\n";
    std::cout << "  Reads and displays GeoTIFF metadata from a TIFF file.\n";
    return 1;
  }

  const auto result = ReadGeoTIFF(std::filesystem::path(argv[1]));
  if (!result.has_value()) {
    std::cout << "Error: " << result.error().message() << '\n';
    return 1;
  }

  const auto& metadata = result.value();

  std::cout << "============= All GeoTIFF Tags =============\n";
  for (const auto& [key, value] : metadata) {
    std::cout << "  0x" << std::hex << std::setw(4) << std::setfill('0')
              << key << std::dec << std::setfill(' ') << "  ";
    PrintMetavalue(value);
    std::cout << '\n';
  }

  std::cout << "\n============= Known GeoTIFF Tags =============\n";
  std::cout << std::left;

  auto print = [&](uint16_t tag, const std::string& name) {
    std::cout << std::setw(40) << name;
    if (metadata.Has(tag)) {
      std::cout << " = ";
      PrintMetavalue(metadata.data().at(tag));
    } else {
      std::cout << " = (not present)";
    }
    std::cout << '\n';
  };

  print(GeoTiffTag::GTModelTypeGeoKey, "GTModelTypeGeoKey");
  print(GeoTiffTag::GTRasterTypeGeoKey, "GTRasterTypeGeoKey");
  print(GeoTiffTag::GTCitationGeoKey, "GTCitationGeoKey");
  print(GeoTiffTag::GeographicTypeGeoKey, "GeographicTypeGeoKey");
  print(GeoTiffTag::GeogGeodeticDatumGeoKey, "GeogGeodeticDatumGeoKey");
  print(GeoTiffTag::GeogPrimeMeridianGeoKey, "GeogPrimeMeridianGeoKey");
  print(GeoTiffTag::GeogLinearUnitsGeoKey, "GeogLinearUnitsGeoKey");
  print(GeoTiffTag::GeogAngularUnitsGeoKey, "GeogAngularUnitsGeoKey");
  print(GeoTiffTag::GeogEllipsoidGeoKey, "GeogEllipsoidGeoKey");
  print(GeoTiffTag::ProjectedCSTypeGeoKey, "ProjectedCSTypeGeoKey");
  print(GeoTiffTag::ProjectionGeoKey, "ProjectionGeoKey");
  print(GeoTiffTag::ProjCoordTransGeoKey, "ProjCoordTransGeoKey");
  print(GeoTiffTag::ProjLinearUnitsGeoKey, "ProjLinearUnitsGeoKey");
  print(GeoTiffTag::VerticalCSTypeGeoKey, "VerticalCSTypeGeoKey");
  print(GeoTiffTag::VerticalDatumGeoKey, "VerticalDatumGeoKey");
  print(GeoTiffTag::VerticalUnitsGeoKey, "VerticalUnitsGeoKey");

  std::cout << "\n============= GeoTIFF Model Tags =============\n";
  print(GeoTiffTiffTag::ModelPixelScaleTag, "ModelPixelScale");
  print(GeoTiffTiffTag::ModelTiepointTag, "ModelTiepoint");
  print(GeoTiffTiffTag::ModelTransformationTag, "ModelTransformation");

  std::cout << "\n========================================\n";
  return 0;
}
