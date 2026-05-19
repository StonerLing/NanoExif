add_executable(read_exif read_exif.cpp)
target_link_libraries(read_exif PRIVATE NanoExif::NanoExif)

add_executable(read_xmp read_xmp.cpp)
target_link_libraries(read_xmp PRIVATE NanoExif::NanoExif)

add_executable(read_geotiff read_geotiff.cpp)
target_link_libraries(read_geotiff PRIVATE NanoExif::NanoExif)
