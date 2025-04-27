file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_utils OBJECT ${SRC_CURRENT})

target_sources(kit_utils PUBLIC
    $<TARGET_OBJECTS:kit_log>
)

target_include_directories(kit_utils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       

target_link_libraries(kit_utils PUBLIC
    shared_dependencies
    kit_log
)
