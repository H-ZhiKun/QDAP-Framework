get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist!")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
        message(WARNING "Required component ${comp} of ${_NAME} not found.")
      else()
        message(WARNING "Optional component ${comp} of ${_NAME} not found.")
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# 仅在 Debug 模式下启用 vld
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/vld-targets.cmake")
    include(${CMAKE_CURRENT_LIST_DIR}/vld-targets.cmake)
  else()
    message(WARNING "vld-targets.cmake not found, skipping vld integration.")
  endif()

  check_required_components(vld)
endif()
