# NanoExif

A **header-only** C++17 library for reading EXIF, XMP, and GeoTIFF metadata
from JPEG and TIFF image files. No external dependencies beyond the C++
standard library.

## Features

- **EXIF** -- parses IFD structures from JPEG (APP1) and TIFF files;
  supports all standard EXIF data types including rationals
- **XMP** -- extracts XML metadata from JPEG APP1 segments or TIFF tag
  0x700 using a built-in lightweight XML parser
- **GeoTIFF** -- reads GeoKey directories, double/ASCII parameter arrays,
  and spatial model tags (ModelTiepoint, ModelPixelScale,
  ModelTransformation)
- **Header-only** -- drop in `src/nanoexif/` and `#include` what you need
- **Result\<T\>** -- all operations return either a value or a descriptive
  error code (no exceptions on parsing failures)

## Quick start

### Read EXIF from a JPEG

```cpp
#include "nanoexif/readers/reader_exif.h"
#include <iostream>

int main(int argc, char** argv) {
  auto result = nne::ReadExif(argv[1]);
  if (!result) {
    std::cerr << "Error: " << result.error().message() << "\n";
    return 1;
  }
  for (const auto& [tag, value] : result.value()) {
    std::cout << "0x" << std::hex << tag << std::dec << "  ";
    nne::PrintMetavalue(value);
    std::cout << "\n";
  }
}
```

### Read XMP from a TIFF

```cpp
#include "nanoexif/readers/reader_xmp.h"
#include <iostream>

int main(int argc, char** argv) {
  auto result = nne::ReadXmp(argv[1]);
  if (!result) {
    std::cerr << "Error: " << result.error().message() << "\n";
    return 1;
  }
  for (const auto& [key, value] : result.value()) {
    std::cout << key << "  ";
    nne::PrintMetavalue(value);
    std::cout << "\n";
  }
}
```

### Read GeoTIFF metadata

```cpp
#include "nanoexif/readers/reader_geotiff.h"
#include <iostream>

int main(int argc, char** argv) {
  auto result = nne::ReadGeoTiff(argv[1]);
  if (!result) {
    std::cerr << "Error: " << result.error().message() << "\n";
    return 1;
  }
  for (const auto& [tag, value] : result.value()) {
    std::cout << "0x" << std::hex << tag << std::dec << "  ";
    nne::PrintMetavalue(value);
    std::cout << "\n";
  }
}
```

## Build

Since NanoExif is **header-only**, there is nothing to compile.  Just add
`src/` to your include path and include the reader headers you need.

### With CMake

```cmake
add_subdirectory(path/to/NanoExif)
target_link_libraries(your_target PRIVATE NanoExif::NanoExif)
```

## Requirements

- C++17 compiler (MSVC 2019+, GCC 7+, Clang 8+)
- No external dependencies

## License

MIT -- see [LICENSE](LICENSE).
