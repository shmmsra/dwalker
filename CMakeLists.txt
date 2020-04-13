# This project we will have two build configurations: Debug, Release
# There is only one target. Output will be an executable

cmake_minimum_required(VERSION 3.14)

# Usage of add_subdirectory vs include: https://cmake.org/pipermail/cmake/2007-November/017897.html
add_subdirectory(src/thirdparty/ph)
add_subdirectory(src/dwalker)

# add_dependencies require the exact target names therefore hardcoding the names here as a workaround
add_dependencies(dwalker phlib)
