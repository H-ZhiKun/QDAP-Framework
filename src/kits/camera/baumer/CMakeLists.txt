
set(KIT_NAME "kit_camera_baumer")
message("---------- Building ${KIT_NAME}  ----------")
file(GLOB_RECURSE CURRENT_SRC 
${CMAKE_CURRENT_SOURCE_DIR}/../CameraManagerBase.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/../CameraManagerBase.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${KIT_NAME} OBJECT ${CURRENT_SRC})

target_compile_definitions(${KIT_NAME} PUBLIC USE_KIT_CAMERA_BAUMER)

message(${THIRD_PARTY_LIB})
target_link_libraries(${KIT_NAME} PUBLIC
    shared_dependencies
    ${THIRD_PARTY_LIB}/baumer_camera/bgapi2_ext_sc.lib 
    ${THIRD_PARTY_LIB}/baumer_camera/bgapi2_genicam.lib 
)

add_custom_target(copy_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRD_PARTY_BIN}/baumer_camera ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)