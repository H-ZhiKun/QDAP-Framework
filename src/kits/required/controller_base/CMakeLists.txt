
file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_controller_base OBJECT ${SRC_CURRENT})
target_sources(kit_controller_base PUBLIC 
$<TARGET_OBJECTS:kit_invoke>
)         

target_link_libraries(kit_controller_base PUBLIC
    shared_dependencies
    kit_invoke
)
