// SPDX-License-Identifier: MIT
//
// Example: read and display XMP metadata from a JPEG/TIFF image.

#include <iomanip>
#include <iostream>
#include <filesystem>

#include "nanoexif/readers/reader_xmp.h"

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace nne;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Usage: read_xmp <image_path>\n";
    std::cout << "  Reads and displays XMP metadata from a JPEG/TIFF file.\n";
    return 1;
  }

  const auto result = ReadXMP(std::filesystem::path(argv[1]));
  if (!result.has_value()) {
    std::cout << "Error: " << result.error().message() << '\n';
    return 1;
  }

  const auto& metadata = result.value();

  std::cout << "============= All XMP Properties =============\n";
  for (const auto& [key, value] : metadata) {
    std::cout << std::setw(40) << std::left << key;
    PrintMetavalue(value);
    std::cout << '\n';
  }

  std::cout << "\n========================================\n";
  return 0;
}
