# This project we will have two build configurations: Debug, Release
# One scheme (default one) corresponding to each target on mac
# There is only one target. Output will be an executable

cmake_minimum_required(VERSION 3.14)

set(DWALKER "dwalker")
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
set(THIRDPARTY_PHLIB_PATH "${PROJECT_ROOT_DIR}/src/thirdparty/ph")

project(${DWALKER})
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /guard:cf")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /guard:cf /DYNAMICBASE")
set(PROJECT_OUTPUT_DIRECTORY "${PROJECT_ROOT_DIR}/build/$<CONFIG>/dwalker/")
add_compile_definitions(_PHLIB_)

# Add DWalker header and source files
set(DWALKER_PATH ${PROJECT_ROOT_DIR}/src/dwalker)
file(GLOB_RECURSE DWALKER_HEADER_FILES
    "${DWALKER_PATH}/include/*.h")
file(GLOB_RECURSE DWALKER_SOURCE_FILES
    "${DWALKER_PATH}/source/*.cpp")

set(DWALKER_OUTPUT_NAME "dwalker")
add_executable(${DWALKER} WIN32 ${DWALKER_HEADER_FILES} ${DWALKER_SOURCE_FILES})

target_link_libraries(${DWALKER}
    "Winhttp.lib"
    "${PROJECT_ROOT_DIR}/build/$<CONFIG>/phlib/phlib.lib")

set_target_properties(${DWALKER} PROPERTIES OUTPUT_NAME ${DWALKER_OUTPUT_NAME})
set_target_properties(${DWALKER} PROPERTIES
    LINK_FLAGS /SUBSYSTEM:CONSOLE
    OUTPUT_NAME ${DWALKER_OUTPUT_NAME}
    # set the build output directory
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
    # Hide the symbols
    CXX_VISIBILITY_PRESET hidden
)

# group source directories
source_group(TREE ${PROJECT_ROOT_DIR}/src FILES ${DWALKER_HEADER_FILES} ${DWALKER_SOURCE_FILES})

# Headers
target_include_directories(${DWALKER} PRIVATE
    "${DWALKER_PATH}/include"
    "${THIRDPARTY_PHLIB_PATH}/phnt/include"
    "${THIRDPARTY_PHLIB_PATH}/phlib/include"
    "${THIRDPARTY_PHLIB_PATH}/phlib/jsonc"
    "${THIRDPARTY_PHLIB_PATH}/phlib/mxml")
