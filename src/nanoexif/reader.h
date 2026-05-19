// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <fstream>
#include <vector>

#include "nanoexif/endian.h"
#include "nanoexif/errors.h"
#include "nanoexif/reader_base.h"
#include "nanoexif/traits.h"
#include "nanoexif/types.h"

namespace nne {
template <MetaFormat meta_format, ImageFormat image_format>
class Reader : ReaderBase<meta_format> {};

}  // namespace nne
