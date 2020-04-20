# This project we will have two build configurations: Debug, Release
# There is only one target. Output will be an executable

cmake_minimum_required(VERSION 3.14)
include(ExternalProject)

ExternalProject_Add(
    xerces
    # URL "https://downloads.apache.org/xerces/c/3/sources/xerces-c-3.2.3.zip"
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/thirdparty/xerces
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/xerces
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/xerces/install
    CMAKE_ARGS
        -DCMAKE_CXX_FLAGS_RELEASE:STRING=/MT
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=/MTd
        -Dnetwork:BOOL=OFF
        -Dtranscoder=windows
        -Dxmlch-type=wchar_t
        -Dmfc-debug:BOOL=OFF
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/xerces/install
)

# Usage of add_subdirectory vs include: https://cmake.org/pipermail/cmake/2007-November/017897.html
add_subdirectory(src/thirdparty/ph)
add_subdirectory(src/dwalker)

# add_dependencies require the exact target names therefore hardcoding the names here as a workaround
add_dependencies(dwalker phlib xerces)
