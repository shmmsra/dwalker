# This project we will have two build configurations: Debug, Release
# One scheme (default one) corresponding to each target on mac
# There is only one target. Output will be an executable

cmake_minimum_required(VERSION 3.14)

set(PHLIB "phlib")
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

project(${PHLIB})
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(EXECUTABLE_TYPE STATIC)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /guard:cf")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /guard:cf /DYNAMICBASE")
set(PROJECT_OUTPUT_DIRECTORY "${PROJECT_ROOT_DIR}/build/$<CONFIG>/phlib/")
add_compile_definitions(_PHLIB_)

# Add PHLib header and source files
set(PHLIB_PATH "${CMAKE_CURRENT_LIST_DIR}")
file(GLOB_RECURSE PHLIB_HEADER_FILES
    "${PHLIB_PATH}/phnt/include/*.h"
    "${PHLIB_PATH}/phlib/*.h"
    "${PHLIB_PATH}/phlib/include/*.h"
    "${PHLIB_PATH}/phlib/jsonc/*.h"
    "${PHLIB_PATH}/phlib/mxml/*.h")
file(GLOB_RECURSE PHLIB_SOURCE_FILES
    "${PHLIB_PATH}/phlib/*.c"
    "${PHLIB_PATH}/phlib/jsonc/*.c"
    "${PHLIB_PATH}/phlib/mxml/*.c")

set(PHLIB_OUTPUT_NAME "phlib")
add_library(${PHLIB}
    ${EXECUTABLE_TYPE}
    ${PHLIB_HEADER_FILES}
    ${PHLIB_SOURCE_FILES})
set_target_properties(${PHLIB} PROPERTIES OUTPUT_NAME ${PHLIB_OUTPUT_NAME})

set_target_properties(${PHLIB} PROPERTIES
    OUTPUT_NAME ${PHLIB_OUTPUT_NAME}
    # set the build output directory
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
    # Hide the symbols
    CXX_VISIBILITY_PRESET hidden
)

# group source directories
source_group(TREE ${PROJECT_ROOT_DIR}/src FILES ${PHLIB_HEADER_FILES} ${PHLIB_SOURCE_FILES})

# Headers
target_include_directories(${PHLIB} PRIVATE
    "${PHLIB_PATH}/phnt/include"
    "${PHLIB_PATH}/phlib/include"
    "${PHLIB_PATH}/phlib/jsonc"
    "${PHLIB_PATH}/phlib/mxml")
