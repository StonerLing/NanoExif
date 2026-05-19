// SPDX-License-Identifier: MIT
//
// NanoExif -- A header-only C++17 library for reading EXIF, XMP, and GeoTIFF
// metadata from JPEG and TIFF image files.
//
// Main include header: pulls in all components.

#pragma once

#include "nanoexif/endian.h"
#include "nanoexif/errors.h"
#include "nanoexif/readers/reader_exif.h"
#include "nanoexif/readers/reader_geotiff.h"
#include "nanoexif/readers/reader_xmp.h"
#include "nanoexif/tags/tags_exif.h"
#include "nanoexif/tags/tags_geotiff.h"
#include "nanoexif/tags/tags_xmp.h"
#include "nanoexif/traits.h"
#include "nanoexif/types.h"

namespace nne {}  // namespace nne
