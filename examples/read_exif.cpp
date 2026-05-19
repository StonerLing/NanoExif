// SPDX-License-Identifier: MIT

#include <iostream>
#include <iomanip>

#include "nanoexif/readers/reader_exif.h"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace nne;

template<typename MetadataT>
void PrintExifMetadata(const MetadataT& metadata) {
    std::cout << "================ EXIF Metadata ================\n";

    std::cout << std::left << std::setw(32) << "PixelXDimension"
              << " = " << metadata.template Get<PixelXDimension>() << '\n';

    std::cout << std::left << std::setw(32) << "PixelYDimension"
              << " = " << metadata.template Get<PixelYDimension>() << '\n';

    std::cout << std::left << std::setw(32) << "ImageDescription"
              << " = " << metadata.template Get<ImageDescription>() << '\n';

    std::cout << std::left << std::setw(32) << "Make"
              << " = " << metadata.template Get<Make>() << '\n';

    std::cout << std::left << std::setw(32) << "Model"
              << " = " << metadata.template Get<Model>() << '\n';

    std::cout << std::left << std::setw(32) << "Orientation"
              << " = " << metadata.template Get<Orientation>() << '\n';

    std::cout << std::left << std::setw(32) << "XResolution"
              << " = " << metadata.template Get<XResolution>() << '\n';

    std::cout << std::left << std::setw(32) << "YResolution"
              << " = " << metadata.template Get<YResolution>() << '\n';

    std::cout << std::left << std::setw(32) << "ResolutionUnit"
              << " = " << metadata.template Get<ResolutionUnit>() << '\n';

    std::cout << std::left << std::setw(32) << "ExifIfdOffset"
              << " = " << metadata.template Get<ExifIfdOffset>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSIfdOffset"
              << " = " << metadata.template Get<GPSIfdOffset>() << '\n';

    std::cout << std::left << std::setw(32) << "CreateDate"
              << " = " << metadata.template Get<CreateDate>() << '\n';

    std::cout << std::left << std::setw(32) << "FocalLength"
              << " = " << metadata.template Get<FocalLength>() << '\n';

    std::cout << std::left << std::setw(32) << "FocalLengthIn35mmFilm"
              << " = " << metadata.template Get<FocalLengthIn35mmFilm>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSLatitudeRef"
              << " = " << metadata.template Get<GPSLatitudeRef>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSLatitude"
              << " = " << metadata.template Get<GPSLatitude>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSLongitudeRef"
              << " = " << metadata.template Get<GPSLongitudeRef>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSLongitude"
              << " = " << metadata.template Get<GPSLongitude>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSAltitudeRef"
              << " = " << metadata.template Get<GPSAltitudeRef>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSAltitude"
              << " = " << metadata.template Get<GPSAltitude>() << '\n';

    std::cout << std::left << std::setw(32) << "GPSMapDatum"
              << " = " << metadata.template Get<GPSMapDatum>() << '\n';

    std::cout << "================================================\n";
}

int main(int argc, char** argv) {
  Reader<EXIF, JPEG> reader(
      "E:/workspace/20260507/datasets/data2/XAG001_0001.JPG");

  auto result = reader.Read();
  if (!result.has_value()) {
    std::cout << result.error().message();
  }

  const auto& metadata = result.value();

  for (const auto& [k, v] : metadata) {
    printf("Key = 0x%04X\n", k);
  }

  PrintExifMetadata(metadata);
}
