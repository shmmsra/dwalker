# Add PHLib header and source files
set(PHLIB "thirdparty/phlib")
set(THIRDPARTY_PHLIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../src/thirdparty/ph")
file(GLOB_RECURSE THIRDPARTY_PHLIB_HEADER_FILES
    "${THIRDPARTY_PHLIB_PATH}/phnt/include/*.h"
    "${THIRDPARTY_PHLIB_PATH}/phlib/*.h"
    "${THIRDPARTY_PHLIB_PATH}/phlib/include/*.h"
    "${THIRDPARTY_PHLIB_PATH}/phlib/jsonc/*.h"
    "${THIRDPARTY_PHLIB_PATH}/phlib/mxml/*.h")
file(GLOB_RECURSE THIRDPARTY_PHLIB_SOURCE_FILES
    "${THIRDPARTY_PHLIB_PATH}/phlib/*.c"
    "${THIRDPARTY_PHLIB_PATH}/phlib/jsonc/*.c"
    "${THIRDPARTY_PHLIB_PATH}/phlib/mxml/*.c")
