set(MODULE_NAME "mod_dev_info")
message("---------- Building ${MODULE_NAME}  ----------")
file(GLOB_RECURSE DEV_INFO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
                                ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${MODULE_NAME} OBJECT ${DEV_INFO_SRC})
target_sources(${MODULE_NAME} PUBLIC 
$<TARGET_OBJECTS:kit_module_base>
)
target_include_directories(${MODULE_NAME} PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR} 
)       

target_link_libraries(${MODULE_NAME} PUBLIC
kit_module_base
)

target_sources(${LIB_MODULES} PUBLIC 
$<TARGET_OBJECTS:${MODULE_NAME}>
)     
target_link_libraries(${LIB_MODULES} PUBLIC ${MODULE_NAME})