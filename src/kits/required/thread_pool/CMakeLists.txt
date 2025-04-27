file(GLOB SRC_CURRENT ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(kit_thread_pool OBJECT ${SRC_CURRENT})
target_include_directories(kit_thread_pool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)       
target_link_libraries(kit_thread_pool PUBLIC
    shared_dependencies
)
