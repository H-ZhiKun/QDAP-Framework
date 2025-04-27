file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_log OBJECT ${SRC_CURRENT})
target_include_directories(kit_log PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(kit_log PUBLIC
    shared_dependencies
)
