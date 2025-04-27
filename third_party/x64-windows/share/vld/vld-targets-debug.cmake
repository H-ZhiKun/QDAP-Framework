#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

set_property(TARGET vld::vld APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(vld::vld PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/vld/vld.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/vld/vld_x64.dll"
  )

list(APPEND _cmake_import_check_targets vld::vld)
list(APPEND _cmake_import_check_files_for_vld::vld "${_IMPORT_PREFIX}/debug/lib/vld/vld.lib" "${_IMPORT_PREFIX}/debug/bin/vld/vld_x64.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
