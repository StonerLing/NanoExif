add_executable(read_exif read_exif.cpp)
target_link_libraries(read_exif PRIVATE NanoExif::NanoExif)
