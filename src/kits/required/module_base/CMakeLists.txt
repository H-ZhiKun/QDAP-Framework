
file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_module_base OBJECT ${SRC_CURRENT})
target_sources(kit_module_base PUBLIC 
$<TARGET_OBJECTS:kit_invoke>
)         
target_include_directories(kit_module_base PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(kit_module_base PUBLIC
    shared_dependencies
    kit_invoke
)
