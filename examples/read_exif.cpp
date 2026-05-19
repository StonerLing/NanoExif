// SPDX-License-Identifier: MIT
//
// Example: read and display EXIF metadata from a JPEG image.

#include <iomanip>
#include <iostream>
#include <string>
#include <filesystem>

#include "nanoexif/readers/reader_exif.h"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace nne;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: read_exif <jpeg_image_path>\n";
    std::cout << "  Reads and displays EXIF metadata from a JPEG file.\n";
    return 1;
  }

  auto result = ReadEXIF(std::filesystem::path(argv[1]));
  if (!result.has_value()) {
    std::cout << "Error: " << result.error().message() << '\n';
    return 1;
  }

  const auto& metadata = result.value();

  std::cout << "============= All EXIF Tags =============\n";
  for (const auto& [key, value] : metadata) {
    std::cout << "  0x" << std::hex << std::setw(4) << std::setfill('0') << key
              << std::dec << std::setfill(' ') << "  ";
    PrintMetavalue(value);
    std::cout << '\n';
  }

  std::cout << "\n============= Known EXIF Tags =============\n";
  std::cout << std::left;

  auto print = [&](auto tag, const std::string& name) {
    std::cout << std::setw(32) << name;
    if (metadata.Has(tag)) {
      std::cout << " = ";
      PrintMetavalue(metadata.data().at(tag));
    } else {
      std::cout << " = (not present)";
    }
    std::cout << '\n';
  };

  print(ExifTag::PixelXDimension, "PixelXDimension");
  print(ExifTag::PixelYDimension, "PixelYDimension");
  print(ExifTag::ImageDescription, "ImageDescription");
  print(ExifTag::Make, "Make");
  print(ExifTag::Model, "Model");
  print(ExifTag::Orientation, "Orientation");
  print(ExifTag::XResolution, "XResolution");
  print(ExifTag::YResolution, "YResolution");
  print(ExifTag::ResolutionUnit, "ResolutionUnit");
  print(ExifTag::CreateDate, "CreateDate");
  print(ExifTag::FocalLength, "FocalLength");
  print(ExifTag::FocalLengthIn35mmFilm, "FocalLengthIn35mmFilm");
  print(ExifTag::GPSLatitudeRef, "GPSLatitudeRef");
  print(ExifTag::GPSLatitude, "GPSLatitude");
  print(ExifTag::GPSLongitudeRef, "GPSLongitudeRef");
  print(ExifTag::GPSLongitude, "GPSLongitude");
  print(ExifTag::GPSAltitudeRef, "GPSAltitudeRef");
  print(ExifTag::GPSAltitude, "GPSAltitude");

  std::cout << "\n========================================\n";
  return 0;
}
