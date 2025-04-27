file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_invoke OBJECT ${SRC_CURRENT})

target_sources(kit_invoke PUBLIC 
$<TARGET_OBJECTS:kit_log>
)
target_include_directories(kit_invoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(kit_invoke PUBLIC
    shared_dependencies
    kit_log
)
