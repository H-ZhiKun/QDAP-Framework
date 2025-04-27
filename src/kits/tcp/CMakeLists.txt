message("---------- Building kit_tcp  ----------")
file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_tcp OBJECT ${SRC_CURRENT})
target_include_directories(kit_tcp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(kit_tcp PUBLIC
    shared_dependencies
)
