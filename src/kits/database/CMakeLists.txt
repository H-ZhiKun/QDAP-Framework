set(KIT_NAME "kit_database")
message("---------- Building ${KIT_NAME}  ----------")
file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(${KIT_NAME} OBJECT ${SRC_CURRENT})
target_include_directories(${KIT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(${KIT_NAME} PUBLIC
    shared_dependencies
)
add_custom_target(copy_postgresql ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${THIRD_PARTY_BIN}/postgresql
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)