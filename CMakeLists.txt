cmake_minimum_required(VERSION 3.5)
project(apone-cpp.image)

set(SOURCE_FILES image/image.cpp image/lodepng.cpp image/packer.cpp)

add_library(image STATIC ${SOURCE_FILES})
target_link_libraries(image PRIVATE coreutils)
target_include_directories(image PUBLIC .)
