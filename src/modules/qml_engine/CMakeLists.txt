set(MODULE_NAME "mod_qml_engine")
message("---------- Building ${MODULE_NAME}  ----------")
file(GLOB_RECURSE SRC_QML_ENGINE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${MODULE_NAME} OBJECT ${SRC_QML_ENGINE})
target_sources(${MODULE_NAME} PUBLIC 
$<TARGET_OBJECTS:kit_module_base>
$<TARGET_OBJECTS:kit_qml_kits>
)
target_link_libraries(${MODULE_NAME} PRIVATE
    shared_dependencies
    kit_module_base
    kit_qml_kits
)

target_sources(${LIB_MODULES} PUBLIC 
$<TARGET_OBJECTS:${MODULE_NAME}>
)     
target_link_libraries(${LIB_MODULES} PUBLIC ${MODULE_NAME})
